mapfile -t sources < <(find src opengl_pipeline -type f -name "*.c")
mapfile -t sources_cpp < <(find src_cpp third_party -type f -name "*.cpp" ! -path "third_party/glfw*")

glfw_sources=(
    third_party/glfw/context.c
    third_party/glfw/init.c
    third_party/glfw/input.c
    third_party/glfw/monitor.c
    third_party/glfw/platform.c
    third_party/glfw/vulkan.c
    third_party/glfw/window.c
    third_party/glfw/x11_init.c
    third_party/glfw/x11_monitor.c
    third_party/glfw/x11_window.c
    third_party/glfw/xkb_unicode.c
    third_party/glfw/posix_time.c
    third_party/glfw/posix_thread.c
    third_party/glfw/posix_poll.c
    third_party/glfw/linux_joystick.c
    third_party/glfw/glx_context.c
    third_party/glfw/egl_context.c
    third_party/glfw/posix_module.c
	third_party/glfw/null_init.c
	third_party/glfw/null_monitor.c
	third_party/glfw/null_window.c
	third_party/glfw/null_joystick.c
	third_party/glfw/osmesa_context.c
)

INCLUDES=(-Isrc -Iinclude -Iinclude/cglm/include -Iopengl_pipeline -Isrc_cpp -Ithird_party/imgui -Ithird_party/imgui/backends -IGLFW)

mkdir -p build

objs=()

for f in "${glfw_sources[@]}"; do
    o="build/glfw_$(basename "$f" .c).o"
    # -D_GLFW_X11 tells GLFW to compile for Linux X11 display server
    gcc -O2 -D_GLFW_X11 -c "$f" -o "$o" "${INCLUDES[@]}"
    #gcc -g -O0 -D_GLFW_X11 -c "$f" -o "$o" "${INCLUDES[@]}"
    objs+=("$o")
done

for f in "${sources[@]}"; do
    o="build/$(basename "$f" .c).o"
    gcc -O2 -c "$f" -o "$o" "${INCLUDES[@]}"
    #gcc -g -O0 -fsanitize=address -fno-omit-frame-pointer -c "$f" -o "$o" "${INCLUDES[@]}"
    objs+=("$o")
done

for f in "${sources_cpp[@]}"; do
    o="build/$(basename "$f" .cpp).o"
    g++ -O2 -Wall -c "$f" -o "$o" "${INCLUDES[@]}"
    #g++ -g -Wall -c "$f" -o "$o" "${INCLUDES[@]}"
    objs+=("$o")
done

g++  "${objs[@]}" -o renderer  -lGL -ldl -lm -lpthread

#-fsanitize=address

