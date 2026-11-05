mapfile -t sources < <(find src opengl_pipeline -type f -name "*.c")
mapfile -t sources_cpp < <(find src_cpp third_party -type f -name "*.cpp" ! -path "third_party/glfw*")

INCLUDES=(-Isrc -Iinclude -Iinclude/cglm/include -Iopengl_pipeline -Isrc_cpp -Ithird_party/imgui -Ithird_party/imgui/backends -IGLFW)

mkdir -p build

objs=()

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

g++  "${objs[@]}" -o renderer  -lGL -ldl -lm -lglfw

#-fsanitize=address

