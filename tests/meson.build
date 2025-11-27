if get_option('tests')
    gtest_dep = dependency('gtest', main: true, required: false)
    if gtest_dep.found()
        # Math tests
        test(
            'math_tests',
            executable(
                'test_math',
                'test_math.cpp',
                include_directories: incdir,
                dependencies: [gtest_dep],
                link_with: [math_lib],
            ),
        )

        # Rendering tests
        test(
            'rendering_tests',
            executable(
                'test_rendering',
                'test_rendering.cpp',
                include_directories: incdir,
                dependencies: [gtest_dep],
                link_with: [render_lib, math_lib],
            ),
        )

        # IO tests
        test(
            'io_tests',
            executable(
                'test_io',
                'test_io.cpp',
                include_directories: incdir,
                dependencies: [gtest_dep],
                link_with: [io_lib, math_lib],
            ),
        )

        # Core tests
        test(
            'core_tests',
            executable(
                'test_core',
                'test_core.cpp',
                include_directories: incdir,
                dependencies: [gtest_dep],
                link_with: [core_lib, math_lib],
            ),
        )
    endif
endif
