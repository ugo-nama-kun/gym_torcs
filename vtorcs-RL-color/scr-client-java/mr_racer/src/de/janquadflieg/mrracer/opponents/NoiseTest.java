/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.opponents;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.classification.*;
import de.janquadflieg.mrracer.telemetry.*;
import de.janquadflieg.mrracer.track.*;

import java.util.*;
/**
 *
 * @author nostromo
 */
public class NoiseTest {
    public static ArrayList<SensorData> buffer = new ArrayList<SensorData>();

    public static SensorData filterNoise(SensorData data) {
        buffer.add(0, data);

        if (buffer.size() > 50) {
            buffer.remove(50);
        }

        ModifiableSensorData result = new ModifiableSensorData();
        result.setData(data);
        result.filterWrongNoisy();

        double[] trackSensor = new double[19];
        //double[] values = new double[buffer.size()];

        for (int i = 0; i < trackSensor.length; ++i) {
            trackSensor[i] = 0;
            for (int j = 0; j < buffer.size(); ++j) {
                trackSensor[i] += buffer.get(j).getTrackEdgeSensors()[i];
                //values[j] = buffer.get(j).getTrackEdgeSensors()[i];
            }
            trackSensor[i] /= buffer.size();
        }

        double[] opponentSensor = new double[36];
        for (int i = 0; i < opponentSensor.length; ++i) {

            // smallest value: 182.012!!!!
            opponentSensor[i] = 200.0;
            if (data.getOpponentSensors()[i] < Observer2011.MAX_DISTANCE_NOISY) {
                //System.out.print(i);
                opponentSensor[i] = 0.0;
                int numValues = 0;
                for (int j = 0; j < buffer.size(); ++j) {
                    double v = buffer.get(j).getOpponentSensors()[i];

                    if (v < Observer2011.MAX_DISTANCE_NOISY) {
                        opponentSensor[i] += v;
                        numValues++;
                    }
                }
                opponentSensor[i] /= numValues;
            }
        }

        result.setTrackEdgeSensors(trackSensor);
        result.setOpponentSensors(opponentSensor);

        return result;
    }

    public static void main(String[] args){
        Random random = new Random(System.currentTimeMillis());
        AngleClassifierWithQuadraticRegression classifier = new AngleClassifierWithQuadraticRegression();
        double noise = 0.02;
        double speedDiff = -20.0; // km/h
        double timeStep = 0.02; // seconds
        double[] sensors = new double[36];
        double[] track = new double[19];
        for(int i=0; i < 9;++i){
            double x = -Math.cos(Math.toRadians(i*10.0))*12;
            double y = Math.sin(Math.toRadians(i*10.0))*12;
            track[i] = Math.sqrt(x*x+y*y);
            System.out.println("track["+i+"] = "+Utils.dTS(track[i]));
        }
        track[9] = 200.0;
        for(int i=10; i < track.length;++i){

        }
        Arrays.fill(sensors, 200.0);
        Observer2011 observer = new Observer2011();
        observer.setTrackModel(TrackDB.create().getByName("Wheel2 (Suzuka)"));

        double distance = 201;
        int offset = 0;

        for(int i=0; i < 2000; ++i){
            System.out.print("["+i+"] ");
            if(i == 447){
                speedDiff *= -1;
                distance = sensors[18];
                offset = i;
            }
            if(i == 650){
                speedDiff *= -1;
                distance = sensors[18];
                offset = i;
            }
            double realDistance = distance + ((i-offset)*timeStep*(speedDiff/3.6));
            sensors[18] = realDistance * (1.0+(random.nextGaussian()*noise));

            for(int k=1; k < 36; ++k){
                random.nextGaussian();
            }

            ModifiableSensorData data = new ModifiableSensorData();
            data.setDistanceFromStartline(4.0);
            data.setAngleToTrackAxis(0.0);
            data.setTrackPosition(0.0);
            data.setTrackEdgeSensors(track);
            data.setOpponentSensors(sensors);

            SensorData filtered = filterNoise(data);

            observer.update(filtered, classifier.classify(data));


            System.out.println(" "+Utils.dTS(realDistance)+" "+
                    Utils.dTS(sensors[18])+" "+
                    Utils.dTS(filtered.getOpponentSensors()[18]));
            
        }
    }
}
