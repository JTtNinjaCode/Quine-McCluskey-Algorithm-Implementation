src_directory="./src"
input_directory="./input"
output_directory="./output"

mkdir -p "$output_directory"
g++ "$src_directory"/main.cpp "$src_directory"/ROBDD.cpp "$src_directory"/OutputInfo.cpp "$src_directory"/InputInfo.cpp -o "$output_directory"/robdd

for i in "$input_directory"/*.pla; do
    file_name=$(basename ${i%.*})
    ./output/robdd "$input_directory"/"$file_name".pla "$output_directory"/"$file_name".dot
    dot -Tpng "$output_directory"/"$file_name".dot -o "$output_directory"/"$file_name".png
done