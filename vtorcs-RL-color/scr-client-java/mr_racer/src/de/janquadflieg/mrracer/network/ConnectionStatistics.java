/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.network;

import de.janquadflieg.mrracer.Utils;

/**
 *
 * @author Jan Quadflieg
 */
public class ConnectionStatistics {
    private int minLatency = Utils.NO_DATA_I;
    private int maxLatency = Utils.NO_DATA_I;
    private int pps = Utils.NO_DATA_I;
    private int avgLatency = Utils.NO_DATA_I;

    public ConnectionStatistics(){
        
    }

    public ConnectionStatistics(int minLatency, int maxLatency, int avgLatency, int pps){
        this.minLatency = minLatency;
        this.maxLatency = maxLatency;
        this.avgLatency = avgLatency;
        this.pps = pps;
    }

    public int getMinLatency(){
        return this.minLatency;
    }

    public String getMinLatencyS(){
        return Utils.iTS(minLatency);
    }

    public int getMaxLatency(){
        return this.maxLatency;
    }

    public String getMaxLatencyS(){
        return Utils.iTS(maxLatency);
    }

    public int getAvgLatency(){
        return this.avgLatency;
    }

    public String getAvgLatencyS(){
        return Utils.iTS(avgLatency);
    }

    public int getPPS(){
        return this.pps;
    }

    public String getPPSS(){
        return Utils.iTS(pps);
    }
}