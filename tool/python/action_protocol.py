"""The versioned process boundary shared by project action handlers."""

import json
import re
from pathlib import Path
from tempfile import NamedTemporaryFile
from uuid import UUID


ACTIONS = ('flash-run', 'export', 'log', 'debug')
INPUT_FIELDS = {
    'none': (),
    'artifact': ('manifest_path', 'artifact_id'),
    'build': ('elf_path', 'debug_file_path'),
    'session': ('session_path', 'session_id'),
}
ERROR_CODES = {
    'INVALID_REQUEST', 'PROVIDER_NOT_FOUND', 'AMBIGUOUS_PROVIDER', 'TOOL_MISSING',
    'INCOMPATIBLE_TARGET', 'ARTIFACT_INVALID', 'ARTIFACT_MISMATCH', 'SYMBOLS_UNAVAILABLE',
    'NO_LOG_RECORD', 'CAPACITY_EXCEEDED', 'ENDPOINT_IN_USE', 'SESSION_BUSY',
    'SESSION_NOT_READY', 'SESSION_MISMATCH', 'TOOL_FAILED', 'CANCELLED', 'TIMEOUT',
    'PROTOCOL_ERROR', 'EXECUTION_ERROR',
}
ID_PATTERN = re.compile(r'[a-z0-9][a-z0-9-]*')
KEY_PATTERN = re.compile(r'[A-Za-z0-9][A-Za-z0-9_.-]*')
HASH_PATTERN = re.compile(r'[a-f0-9]{64}')


class ActionError(ValueError):
    def __init__(self, code, message, *, exit_code=1, data=None):
        super().__init__(f'{code}: {message}')
        self.code = code
        self.exit_code = exit_code
        self.data = data or {}


def _object(pairs):
    obj = dict(pairs)
    if len(obj) != len(pairs):
        raise ValueError('Duplicate JSON field')
    return obj


def _reject_constant(value):
    raise ValueError(f'Invalid JSON constant {value}')


def read_json(path):
    return json.loads(Path(path).read_text(encoding='utf-8'), object_pairs_hook=_object,
                      parse_constant=_reject_constant)


def write_json(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    staged = None
    try:
        with NamedTemporaryFile(mode='w', encoding='utf-8', dir=path.parent, delete=False) as stream:
            staged = Path(stream.name)
            json.dump(value, stream, ensure_ascii=False, indent=2, allow_nan=False)
            stream.write('\n')
        staged.replace(path)
    finally:
        if staged is not None:
            staged.unlink(missing_ok=True)


def exact(obj, *fields, optional=()):
    if not isinstance(obj, dict) or set(fields) - obj.keys() or obj.keys() - set((*fields, *optional)):
        raise ValueError(f'Expected fields {", ".join(fields)}'
                         + (f' (optional {", ".join(optional)})' if optional else ''))
    return obj


def string(value, name, *, pattern=None):
    if (type(value) is not str or not value or '\0' in value
            or (pattern is not None and not pattern.fullmatch(value))):
        raise ValueError(f'Invalid {name}: {value!r}')
    return value


def absolute(value, name):
    if not Path(string(value, name)).is_absolute():
        raise ValueError(f'{name} must be an absolute path')
    return value


def uuid(value, name):
    if str(UUID(string(value, name))) != value:
        raise ValueError(f'Invalid {name}: {value!r}')
    return value


def version(obj):
    if type(obj['schema_version']) is not int or obj['schema_version'] != 1:
        raise ValueError('Unsupported schema_version')


def endpoint(value):
    exact(value, 'host', 'port')
    string(value['host'], 'host', pattern=re.compile(r'[A-Za-z0-9.:-]+'))
    if type(value['port']) is not int or not 1 <= value['port'] <= 65535:
        raise ValueError('Port must be an integer between 1 and 65535')
    return value


def load_request(path):
    return validate_request(read_json(path))


def validate_request(request):
    exact(request, 'schema_version', 'request_id', 'operation', 'action', 'provider', 'project_root',
          'sdk_root', 'board', 'target', 'build_type', 'build_dir', 'input', 'tool_overrides', 'options')
    version(request)
    uuid(request['request_id'], 'request_id')
    action, operation = request['action'], request['operation']
    if action not in ACTIONS or operation not in ('execute', 'check', 'stop'):
        raise ValueError('Unknown action or operation')
    string(request['provider'], 'provider', pattern=ID_PATTERN)
    absolute(request['project_root'], 'project_root')
    absolute(request['sdk_root'], 'sdk_root')
    board = exact(request['board'], 'id', 'chip', 'core_id', 'manifest_path')
    string(board['id'], 'board.id', pattern=re.compile(r'board/[a-z0-9][a-z0-9-]*'))
    string(board['core_id'], 'board.core_id')
    if board['chip'] is not None:
        string(board['chip'], 'board.chip')
    absolute(board['manifest_path'], 'board.manifest_path')
    string(request['target'], 'target', pattern=KEY_PATTERN)
    if request['build_type'] is not None or operation != 'check':
        string(request['build_type'], 'build_type', pattern=KEY_PATTERN)
    if request['build_dir'] is not None:
        absolute(request['build_dir'], 'build_dir')
    if not isinstance(request['tool_overrides'], dict):
        raise ValueError('Expected a tool_overrides object')
    for name, value in request['tool_overrides'].items():
        string(name, 'tool ID', pattern=KEY_PATTERN)
        absolute(value, 'tool path')
    input_data = request['input']
    kind = input_data.get('kind') if isinstance(input_data, dict) else None
    if type(kind) is not str or kind not in INPUT_FIELDS:
        raise ValueError('Unknown action input kind')
    exact(input_data, 'kind', *INPUT_FIELDS[kind])
    if kind == 'artifact':
        absolute(input_data['manifest_path'], 'manifest_path')
        string(input_data['artifact_id'], 'artifact_id', pattern=HASH_PATTERN)
    elif kind == 'build':
        absolute(input_data['elf_path'], 'elf_path')
        if input_data['debug_file_path'] is not None:
            absolute(input_data['debug_file_path'], 'debug_file_path')
    elif kind == 'session':
        absolute(input_data['session_path'], 'session_path')
        uuid(input_data['session_id'], 'session_id')
    options = request['options']
    if operation == 'check':
        if kind != 'none':
            raise ValueError('Tool checks require an empty input')
        allowed = ('input_kind', 'profile') if action == 'debug' else ('input_kind',)
        exact(options, optional=allowed)
        if 'input_kind' in options and options['input_kind'] not in ('none', 'build', 'artifact'):
            raise ValueError('Unknown options.input_kind')
        if 'profile' in options:
            string(options['profile'], 'profile', pattern=ID_PATTERN)
    elif operation == 'stop':
        if action != 'debug' or kind != 'session':
            raise ValueError('Stop requires a debug session')
        exact(options)
    elif action == 'debug':
        exact(options, 'profile', 'mode', 'server', 'client', 'gdb_path', 'partition', 'endpoint')
        string(options['profile'], 'profile', pattern=ID_PATTERN)
        absolute(options['gdb_path'], 'gdb_path')
        if options['mode'] not in ('start', 'attach') or options['server'] not in ('owned', 'external'):
            raise ValueError('Invalid debug mode or server ownership')
        if options['client'] not in ('cli', 'ide') or kind not in ('build', 'artifact'):
            raise ValueError('Invalid debug client or input')
        if kind == 'artifact':
            string(options['partition'], 'partition', pattern=KEY_PATTERN)
        elif options['partition'] is not None:
            raise ValueError('Build debug does not specify an archive partition')
        if options['endpoint'] is not None or options['server'] == 'external' or options['client'] == 'ide':
            endpoint(options['endpoint'])
    elif action == 'flash-run':
        exact(options, optional=('duration_ms',))
        if kind != 'artifact':
            raise ValueError('flash-run requires a firmware artifact')
        if 'duration_ms' in options and (type(options['duration_ms']) is not int or options['duration_ms'] <= 0):
            raise ValueError('duration_ms must be a positive integer')
    else:
        exact(options)
        if action == 'export' and kind != 'artifact':
            raise ValueError('export requires a firmware artifact')
        if action == 'log' and kind not in ('none', 'artifact'):
            raise ValueError('log requires a live source or an archive')
    return request


def result(request, *, data=None, code='OK', message=''):
    status = {'OK': 'ok', 'CANCELLED': 'cancelled'}.get(code, 'error')
    return {'schema_version': 1, 'request_id': request['request_id'], 'status': status,
            'code': code, 'message': message, 'data': {} if data is None else data}


def validate_result(value, request, returncode):
    exact(value, 'schema_version', 'request_id', 'status', 'code', 'message', 'data')
    version(value)
    operation, action = request['operation'], request['action']
    if value['request_id'] != request['request_id'] or type(value['message']) is not str:
        raise ValueError('Result does not match the request')
    code = string(value['code'], 'result code')
    status = {'OK': 'ok', 'CANCELLED': 'cancelled'}.get(code, 'error')
    if ((status == 'ok') != (returncode == 0) or value['status'] != status
            or (code != 'OK' and (code not in ERROR_CODES or not value['message']))):
        raise ValueError('Result status, code and process exit disagree')
    data = value['data']
    if status != 'ok':
        exact(data, optional=('record_path', 'tool_exit_code', 'completed_partitions', 'prerequisite',
                              'partition', 'size', 'capacity', 'load_address', 'address'))
        if 'tool_exit_code' in data and (type(data['tool_exit_code']) is not int or not data['tool_exit_code']):
            raise ValueError('Invalid tool exit code')
        if code == 'TOOL_FAILED':
            raw = data.get('tool_exit_code')
            if raw is None or returncode != (128 - raw if raw < 0 else raw):
                raise ValueError('Tool failure does not match the process exit')
        return value
    if operation == 'check':
        exact(data, 'available', 'missing')
        if (type(data['available']) is not bool or not isinstance(data['missing'], list)
                or data['available'] != (not data['missing'])):
            raise ValueError('Invalid tool check result')
        for missing in data['missing']:
            exact(missing, 'id', 'message')
            string(missing['id'], 'tool ID')
            string(missing['message'], 'tool diagnostic')
    elif action == 'debug':
        exact(data, 'session_id', *(('stopped',) if operation == 'stop' else ()))
        uuid(data['session_id'], 'session_id')
        if operation == 'stop' and (type(data['stopped']) is not bool
                                    or data['session_id'] != request['input']['session_id']):
            raise ValueError('Invalid stop result')
    elif action == 'log':
        exact(data, 'source', 'artifact_id', 'record_path')
        if request['input']['kind'] == 'none':
            if data != {'source': 'live', 'artifact_id': None, 'record_path': None}:
                raise ValueError('Invalid live log result')
        else:
            if data['source'] != 'recorded' or data['artifact_id'] != request['input']['artifact_id']:
                raise ValueError('Invalid recorded log result')
            absolute(data['record_path'], 'record_path')
    else:
        fields = {'flash-run': ('partitions', 'record_path', 'virtual_ms'),
                  'export': ('archive_path', 'sha256')}
        exact(data, 'artifact_id', *fields[action])
        if data['artifact_id'] != request['input'].get('artifact_id'):
            raise ValueError('Result reports a different firmware artifact')
        if action == 'flash-run':
            if not isinstance(data['partitions'], list) or not data['partitions']:
                raise ValueError('flash-run must report the loaded partitions')
            for part in data['partitions']:
                exact(part, 'name', 'address', 'sha256')
                string(part['name'], 'partition name')
                string(part['address'], 'written address', pattern=re.compile(r'0x(?:0|[1-9a-f][a-f0-9]*)'))
                string(part['sha256'], 'written hash', pattern=HASH_PATTERN)
            if data['record_path'] is not None or data['virtual_ms'] is not None:
                absolute(data['record_path'], 'record_path')
                if type(data['virtual_ms']) is not int or data['virtual_ms'] <= 0:
                    raise ValueError('Invalid virtual runtime')
        else:
            absolute(data['archive_path'], 'archive_path')
            string(data['sha256'], 'archive hash', pattern=HASH_PATTERN)
    return value


def validate_session(value, request, profile, elf_path, debug_file_path):
    options, input_data, board = request['options'], request['input'], request['board']
    expected = {
        'schema_version': 1, 'state': 'ready', 'request_id': request['request_id'],
        'provider': request['provider'], 'board_id': board['id'], 'core_id': board['core_id'],
        'target': request['target'], 'build_type': request['build_type'], 'profile': options['profile'],
        'mode': options['mode'], 'server': options['server'], 'after_connect': profile['after_connect'],
        'gdb_path': options['gdb_path'], 'elf_path': str(elf_path),
        'debug_file_path': str(debug_file_path) if debug_file_path else None,
        'artifact_id': input_data.get('artifact_id'), 'partition': options['partition'],
    }
    exact(value, *expected, 'session_id', 'endpoint')
    version(value)
    if any(value[key] != expected[key] for key in expected):
        raise ValueError('Session does not match the requested profile, target or symbols')
    uuid(value['session_id'], 'session_id')
    actual_endpoint = endpoint(value['endpoint'])
    if options['endpoint'] is not None and actual_endpoint != options['endpoint']:
        raise ValueError('Session bound a different endpoint')
    return value
