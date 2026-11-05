mapfile -t sources < <(find src opengl_pipeline -type f -name "*.c")
mapfile -t sources_cpp < <(find src_cpp third_party -type f -name "*.cpp")

INCLUDES=(-Isrc -Iinclude -Iinclude/cglm/include -Iopengl_pipeline -Isrc_cpp -Ithird_party/imgui -Ithird_party/imgui/backends)

mkdir -p build

objs=()

for f in "${sources[@]}"; do
    o="build/$(basename "$f" .c).o"
    gcc -g -O0 -fsanitize=address -fno-omit-frame-pointer -c "$f" -o "$o" "${INCLUDES[@]}"
    objs+=("$o")
done

for f in "${sources_cpp[@]}"; do
    o="build/$(basename "$f" .cpp).o"
    g++ -g -Wall -c "$f" -o "$o" "${INCLUDES[@]}"
    objs+=("$o")
done

g++ -fsanitize=address "${objs[@]}" -o renderer -lglfw -lGL -ldl -lm

