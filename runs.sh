path=../datasets/

make clean;
make;

input=$1
eps=$2
minpts=$3
m=$4
M=$5

output=output_$1\_EPS=$eps\_Minpts=$minpts\_m=$m\_M=$M.txt
debug=debug_$1\_EPS=$eps\_Minpts=$minpts\_m=$m\_M=$M.txt
neighbour=neighbour_$1\_EPS=$eps\_Minpts=$minpts\_m=$m\_M=$M.txt

./output $path$input $eps $minpts $m $M $output
