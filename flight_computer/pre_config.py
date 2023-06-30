import os
Import("env")

env.Append(
    CFLAGS=["-std=c17"],
    CCFLAGS=[
        # cpu
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mthumb-interwork",
        # floats
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",

        "-ffunction-sections",
        "-fdata-sections",
        "-fno-common",
        "-fmessage-length=0",
        "-fdiagnostics-color=always",
        "-fstack-usage",

        "-Wall",
        "-Wimplicit-fallthrough",
        "-Wshadow",
        "-Wdouble-promotion",
        "-Wundef",
        "-Wformat=2",
        "-Wformat-truncation=2",
        "-Wformat-overflow",
        "-Wformat-signedness",

        "-Werror",

        "-Wno-packed-bitfield-compat"
    ],
    CXXFLAGS=[
        "-std=c++20",
        "-frtti",
        # Disable volatile warnings of type "compound assignment with 'volatile'-qualified left operand is deprecated [-Wvolatile]"
        # This is heavily used by STM libraries and creates too much noise when compiling
        # Eventually this flag should be set only for library files
        "-Wno-volatile"],
    LINKFLAGS=[
        # cpu
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mthumb-interwork",
        # floats
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",

        "-Wl,-gc-sections,--print-memory-usage,-Map,.pio/build/genericSTM32F411CE/output.map"
    ]
)


# Configure emfat_file.c separately
def emfat_file_config(env, node):
    """
    `node.name` - a name of File System Node
    `node.get_path()` - a relative path
    `node.get_abspath()` - an absolute path
    """

    if "emfat_file.c" not in node.name:
        return node

    return env.Object(
        node,
        CCFLAGS=env["CCFLAGS"] + ["-Wno-missing-braces",
                                  "-Wno-int-conversion",
                                  "-Wno-error"]
    )
env.AddBuildMiddleware(emfat_file_config)

# include toolchain paths
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)
# override compilation DB path
env.Replace(COMPILATIONDB_PATH=os.path.join(
    "$BUILD_DIR", "compile_commands.json"))

print(env["CFLAGS"])
print(env["CCFLAGS"])
print(env["CXXFLAGS"])

# Dump build environment (for debug)
# print(env.Dump())
