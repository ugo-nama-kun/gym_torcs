WARNING: We had some trouble with flanagan.jar and very old Java installations,
namely jdk 1.6.0_10 (from 2008). Please make sure to use a jdk version which is 
not too old
-> everything from 1.6.0 Update 22 (October 2010) onwards will definitly work 

Running the controller using the provided client is a piece of cake:

Open a console and change to the directory containing MrRacer.jar and the lib directory. Type in the following command (change port, host, etc at your own will):

Linux / Unix / Mac
java -cp .:MrRacer.jar champ2011client.Client de.janquadflieg.mrracer.controller.MrRacer2011 host:127.0.0.1 port:3001 stage:0 trackName:someOvalTrack maxSteps:100000


Windows (; instead of : in the classpath):
java -cp .;MrRacer.jar champ2011client.Client de.janquadflieg.mrracer.controller.MrRacer2011 host:127.0.0.1 port:3001 stage:0 trackName:someOvalTrack maxSteps:100000

At the end of the warmup stage, the controller saves a track model to a file "trackname.saved_model" in the directory from which the controller has been executed.
At the start of an evaluation, the controller then loads all files with the extension .saved_model it can find in the current directory and if the current stage is qualifying or race, it then selects the one matching the given track name.
A human readable version of the track models in the current directory can be generated using the command
java -cp .;MrRacer.jar de.janquadflieg.mrracer.track.PrintTrackModels
We now also test a number of parameter sets during the warmup and save the one
selected as "trackname.params".