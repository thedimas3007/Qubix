from rich import inspect

Import("env")

board = env.BoardConfig()

def as_int(v, default=0):
    if v is None:
        return default
    try:
        return int(str(v).rstrip("L"), 0)
    except Exception:
        return default

inspect(board)
mcu        = board.get("build.mcu")
f_cpu      = as_int(board.get("build.f_cpu"))
flash_max  = as_int(board.get("upload.maximum_size"))
ram_max    = as_int(board.get("upload.maximum_ram_size"))
board_id   = env["BOARD"]
platform   = env.PioPlatform().name

env.Append(
    CPPDEFINES=[
        ("PIO_PLATFORM",        f'\\"{platform}\\"'),
        ("PIO_BOARD",           f'\\"{board_id}\\"'),
        ("HW_MCU",              f'\\"{mcu or ''}\\"'),
        ("HW_F_CPU",            f_cpu),
        ("HW_FLASH_BYTES",      flash_max),
        ("HW_RAM_BYTES",        ram_max),
    ]
)
