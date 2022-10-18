# ARQApplicationWithSocket

1. Download and copy ARQApplicationWithSocket folder inside Scratch/ the scratch folder must contain only folders not files. autherwise, the programm will    not execute.
2. in file sourceApplication.cc initialize ploss=0.0 and lb=0 inside the constructor
3. in main-project.cc file change the mobilitymodel according to mobilityModel that you have where the destination must be known for each execution. for example execution 1: distance between sourceNode and SinkNode = 10 m, execution 2: distance = 20 m ...
4. It is recommentded for each execution to clear the cach memory:
    sudo sh -c "sync; echo 1 > /proc/sys/vm/drop_caches"
    sudo sh -c "sync; echo 2 > /proc/sys/vm/drop_caches"
    sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
5. for each distination, execute the programm for 9 times for example and take the average result.
