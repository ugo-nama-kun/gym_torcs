/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.telemetry;

/**
 *
 * @author Jan Quadflieg
 */
public interface TelemetryListener {
    
    /**
     * Called, when a new lap has started.
     */
    public void newLap();

    /**
     * Called, when all data has been delted.
     */
    public void cleared();

    /**
     * Called, when new data has been added.
     * @param mode Indicates, wether the underlying telemetry object is in
     * recording or playback mode.
     */
    public void newData(SensorData data, Action a, String log, Telemetry.Mode mode);


    public void modeChanged(int newMode);
}