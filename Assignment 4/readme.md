## To compile the program, 
> To remove the executables, run 'make clean'
> To compile the source files SenderGBN and ReceiverGBN run 'make' command

## To run the ReceiverGBN, 
```bash
./ReceiverGBN -p 8000 -n 400 -e 0.00001 -d>receiver_out
```

## To run the SenderGBN, 
```bash
./SenderGBN -s aditya -p 8000 -l 512 -r 10 -n 400 -w 10 -f 10 -d>sender_out
```