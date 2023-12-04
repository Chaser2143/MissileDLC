# add_executable(lab8_m1.elf main_m1.c missile.c)
# target_link_libraries(lab8_m1.elf ${330_LIBS} interrupts intervalTimer touchscreen)
# set_target_properties(lab8_m1.elf PROPERTIES LINKER_LANGUAGE CXX)

# add_executable(lab8_m2.elf main_m2.c missile.c gameControl.c plane.c)
# target_link_libraries(lab8_m2.elf ${330_LIBS} interrupts intervalTimer touchscreen)
# set_target_properties(lab8_m2.elf PROPERTIES LINKER_LANGUAGE CXX)

add_subdirectory(sounds)
add_executable(lab9.elf main_m3.c missile.c gameControl.c plane.c sound.c timer_ps.c powerup.c)
target_link_libraries(lab9.elf ${330_LIBS} interrupts intervalTimer touchscreen sounds)
set_target_properties(lab9.elf PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(lab9.elf PUBLIC LAB8_M3)