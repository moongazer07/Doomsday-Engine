if (NOT WIN32)
    math (EXPR _bits "8*${CMAKE_SIZEOF_VOID_P}")
else ()
    # Only 32-bit Windows builds supported.
    set (_bits 32)
endif ()
set (ARCH_BITS "${_bits}" CACHE STRING "CPU architecture bits (32/64)")
set (_bits)

if (ARCH_BITS EQUAL 64)
    add_definitions (-DDENG_64BIT_HOST)
endif ()
