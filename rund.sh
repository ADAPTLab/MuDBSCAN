path=../datasets/

input=$1
eps=$2
minpts=$3

nodes=$4
hostfile=$5
m=$6
M=$7

output=output_$1\_EPS=$eps\_Minpts=$minpts\_nodes=$nodes\_m=$m\M=$M.txt
make clean
make

mpirun -np $nodes --map-by node --hostfile $hostfile ./output $path$input $eps $minpts $m $M $output
