"""Monitor helpers evaluated by Renode's embedded IronPython."""

import sys

import clr

clr.AddReference("System.Security.Cryptography")
from System import Convert
from System.IO import File
from System.Security.Cryptography import SHA256
from System.Threading import Timer, TimerCallback


_gdb_timers = {}


def _emit(message):
    sys.stdout.write(message + '\n')
    sys.stdout.flush()


def mc_fly_watch_gdb(port):
    machine = monitor.Machine
    connected = [False]

    def poll(_):
        active = machine.IsGdbConnectedToServer(port)
        if active and not connected[0]:
            connected[0] = True
            _emit('FLY_GDB_CONNECTED')
        elif not active and connected[0]:
            _gdb_timers.pop(port).Dispose()
            _emit('FLY_GDB_DISCONNECTED')

    _gdb_timers[port] = Timer(TimerCallback(poll), None, 50, 50)
    _emit('FLY_GDB_WATCH_READY')


def mc_fly_verify_binary(address, size, expected):
    bus = monitor.Machine.SystemBus
    hasher = SHA256.Create()
    try:
        actual = ''.join('%02x' % byte for byte in hasher.ComputeHash(bus.ReadBytes(address, size)))
    finally:
        hasher.Dispose()
    if actual != expected:
        raise ValueError('Memory digest mismatch at %x: %s' % (address, actual))
    _emit('FLY_BIN_VERIFIED %x %s' % (address, actual))


def mc_fly_run(duration_ms, completion_file):
    duration_ms = int(duration_ms)
    _emit('FLY_RENODE_RUN_STARTED')
    remaining = duration_ms
    while duration_ms == 0 or remaining > 0:
        chunk = min(50, remaining) if duration_ms else 50
        if not monitor.Parse('emulation RunFor "%.3f"' % (chunk / 1000.0)):
            raise RuntimeError('Could not advance Renode virtual time')
        mc_fly_drain_rtt()
        if duration_ms:
            remaining -= chunk
    # Completion must survive truncation or interleaving in the console log.
    File.WriteAllText(completion_file, str(duration_ms))
    _emit('FLY_RENODE_RUN_COMPLETE')


def mc_fly_drain_rtt():
    # SEGGER's 32-bit RTT descriptor for the first target-to-host channel.
    # Read both ring spans before advancing RdOff, exactly as a probe does.
    bus = monitor.Machine.SystemBus
    cpu = next(iter(bus.GetCPUs()))
    found, addresses = bus.TryGetAllSymbolAddresses('_SEGGER_RTT', context=cpu)
    if not found:
        return
    block = next(iter(addresses))
    signature = ''.join(chr(v) for v in bus.ReadBytes(block, 10))
    if signature != 'SEGGER RTT':
        return
    buffer = bus.ReadDoubleWord(block + 28)
    size = bus.ReadDoubleWord(block + 32)
    write = bus.ReadDoubleWord(block + 36)
    read = bus.ReadDoubleWord(block + 40)
    if not buffer or not (0 <= read < size <= 1024 * 1024 and 0 <= write < size):
        return
    if read == write:
        return
    if read < write:
        _emit('FLY_RTT64 ' + Convert.ToBase64String(bus.ReadBytes(buffer + read, write - read)))
    else:
        _emit('FLY_RTT64 ' + Convert.ToBase64String(bus.ReadBytes(buffer + read, size - read)))
        if write:
            _emit('FLY_RTT64 ' + Convert.ToBase64String(bus.ReadBytes(buffer, write)))
    bus.WriteDoubleWord(block + 40, write)
