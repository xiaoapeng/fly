import struct
import binascii

# 定义常量
FC_PART_NAME_MAX_SIZE = 12
FC_FIRMWARE_NAME_MAX_SIZE = 32
FC_DESCRIPTION_LEN = 64
FC_MAGIC_A = 0xAA5555AA
FC_MAGIC_B = 0x88777788

# 定义结构体的格式（小端序），注意 oter_data 为动态长度部分
struct_format = '<IIIIII12s32s64s'

# 构造结构体数据
def create_factory_data(software_len, factory_len, crc32, software_version, part_name, firmware_name, generated_description, oter_data):
    # Ensure strings are of correct length
    part_name = part_name.ljust(FC_PART_NAME_MAX_SIZE, '\0')
    firmware_name = firmware_name.ljust(FC_FIRMWARE_NAME_MAX_SIZE, '\0')
    generated_description = generated_description.ljust(FC_DESCRIPTION_LEN, '\0')
    
    # Pack the data
    packed_data = struct.pack(
        struct_format,
        FC_MAGIC_A,
        FC_MAGIC_B,
        software_len,
        factory_len,
        crc32,
        software_version,
        part_name.encode('utf-8'),
        firmware_name.encode('utf-8'),
        generated_description.encode('utf-8')
    )
    
    # Add oter_data at the end
    packed_data += oter_data
    
    return packed_data

# 计算CRC32
def calculate_crc32(data, initial_crc=~(0xFFFFFFFF)):
    crc = initial_crc
    crc = binascii.crc32(data, crc) & 0xFFFFFFFF
    return crc

# 读取二进制文件并添加结构体数据
def append_to_bin_file(file_path, software_version, part_name, firmware_name, generated_description, oter_data):
    # Read the existing file content
    with open(file_path, 'rb') as f:
        file_data = f.read()

    # Calculate padding for 8-byte alignment using 0xFF
    padding_length = (8 - (len(file_data) % 8)) % 8
    padding = b'\xFF' * padding_length

    # Create initial factory data with placeholder CRC32
    software_len = len(file_data) + padding_length  # 分区开始位置到本数据结构的长度
    factory_len = struct.calcsize(struct_format) + len(oter_data)
    
    initial_factory_data = create_factory_data(
        software_len,
        factory_len,
        0xFFFFFFFF,  # 初始CRC32值
        software_version,
        part_name,
        firmware_name,
        generated_description,
        oter_data
    )

    # Combine file data, padding, and initial factory data for CRC32 calculation
    combined_data = file_data + padding + initial_factory_data

    # Calculate the CRC32 including the padding
    crc32 = calculate_crc32(combined_data)
    # Recreate the factory data with the calculated CRC32
    final_factory_data = create_factory_data(
        software_len,
        factory_len,
        crc32,
        software_version,
        part_name,
        firmware_name,
        generated_description,
        oter_data
    )

    # Combine the file data, padding, and final factory data
    new_file_data = file_data + padding + final_factory_data

    # Calculate padding for 16-byte alignment for the final file
    final_padding_length = (16 - (len(new_file_data) % 16)) % 16
    final_padding = b'\xFF' * final_padding_length

    # Write the new file content with final padding
    with open(file_path, 'wb') as f:
        f.write(new_file_data + final_padding)

def append_factory_data_to_bin(file_path, software_version, part_name, firmware_name, generated_description, oter_data=b''):
    """
    向二进制文件末尾添加FactoryData结构体
    :param file_path: 二进制文件路径
    :param software_version: 软件版本 (uint32)
    :param part_name: 分区名称
    :param firmware_name: 固件名称
    :param generated_description: 描述信息
    :param oter_data: 其他数据 (bytes)
    """
    append_to_bin_file(file_path, software_version, part_name, firmware_name, generated_description, oter_data)
