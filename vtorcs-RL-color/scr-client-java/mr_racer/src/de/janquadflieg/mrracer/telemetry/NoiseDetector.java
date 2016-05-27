/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.telemetry;

import java.util.ArrayList;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.opponents.Observer2011;

/**
 *
 * @author quad
 */
public class NoiseDetector {
    /** Buffer for moving average noise filtering. */
    private ArrayList<SensorData> buffer = new ArrayList<SensorData>(20);

    private ArrayList<double[]> bufferW = new ArrayList<double[]>();

    private boolean firstPacket = true;
    private boolean secondPacket = true;

    private double width = 0.0;

    private boolean noisy = false;

    private static final boolean TEXT_DEBUG = false;

    public void clearBuffer(){
        buffer.clear();
    }

    public double getWidth(){
        return width;
    }

    public boolean isNoisy(){
        return noisy;
    }

    public SensorData filterNoise(SensorData data) {
        buffer.add(0, data);

        if (buffer.size() > 25) {
            buffer.remove(25);
        }

        ModifiableSensorData result = new ModifiableSensorData();
        result.setData(data);
        result.filterWrongNoisy();

        double[] trackSensor = new double[19];
        //double[] values = new double[buffer.size()];

        for (int i = 0; i < trackSensor.length; ++i) {
            trackSensor[i] = 0;
            for (int j = 0; j < Math.min(10, buffer.size()); ++j) {
                trackSensor[i] += buffer.get(j).getTrackEdgeSensors()[i];
                //values[j] = buffer.get(j).getTrackEdgeSensors()[i];
            }
            trackSensor[i] /= Math.min(10, buffer.size());
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

    public void update(SensorData data){
        if(firstPacket){
            double[] sensors = data.getTrackEdgeSensors();
            width = sensors[0]+sensors[18];
            firstPacket = false;
            return;
        }
        if(secondPacket){
            double[] sensors = data.getTrackEdgeSensors();
            noisy = sensors[0]+sensors[18] != width;
            secondPacket = false;

            if(TEXT_DEBUG && noisy){
                System.out.println("Noise detected");
            }
        }

        if(data.getCurrentLapTime() < 0.0 && noisy){
            double[] sensors = data.getTrackEdgeSensors();
            double[] copy = new double[sensors.length];
            System.arraycopy(sensors, 0, copy, 0, sensors.length);
            bufferW.add(copy);

            double calcWidth = 0;
            
            for(double[] d: bufferW){
                calcWidth += d[0]+d[18];
            }
            width = calcWidth / bufferW.size();
            //System.out.println("Buffer: "+buffer.size()+", "+Utils.dTS(width)+" - "+Utils.dTS(sensors[0])+", "+Utils.dTS(sensors[18]));
        }
    }

    public void reset(){
        firstPacket = true;
        secondPacket = true;
        bufferW.clear();
        buffer.clear();
    }
}