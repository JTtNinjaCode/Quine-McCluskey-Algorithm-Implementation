src_directory="./src"
input_directory="./input"
output_directory="./output"

mkdir -p "$output_directory"
g++ "$src_directory"/B11009054.cpp "$src_directory"/QM.cpp "$src_directory"/TermsInfo.cpp -o "$output_directory"/B11009054

for i in "$input_directory"/*.pla; do
    file_name=$(basename ${i%.*})
    ./output/B11009054 "$input_directory"/"$file_name".pla "$output_directory"/"$file_name"_output.pla
done
