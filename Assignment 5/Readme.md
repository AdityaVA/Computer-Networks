## To compile the program, 
> Run ```make clean``` to remove the executalbes
> Run ```make``` to compile all the files into an executable ospf

## To run the code type 
```
./ospf -i id -f infile -o outfile -h hi -a lsai -s spfi
```
or 
```
./ospf -n num_nodes -f infile -o outfile -h hi -a lsai -s spfi
```
The values specified in the command line are:
> -i id: Node identifier value (i)

> -n num_nodes: Number of nodes to start (0 to n-1)

> -f infile: Input file

> -o outfile: Output file

> -h hi: HELLOINTERVAL (in seconds)

> -a lsai: LSAINTERVAL (in seconds)

> -s spfi: SPFINTERVAL (in seconds)