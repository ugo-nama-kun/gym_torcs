/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.delbrueg.steering.benchmark;

import java.util.LinkedList;
import java.util.List;

/**
 * This class represents a benchmark for a SteeringBehaviour that has to reach target_positions. 
 * It works by saving a list of all positions and target positions. After the list
 * is ready (after 1 lap) getStatistics() is called, which computes some statistical data.
 * @author Tim
 */
public class Benchmark {

    List<BenchmarkEntry> entries = new LinkedList<BenchmarkEntry>();
    double target_pos = 0;
    double old_target_pos = 0;
    double old_old_target_pos = 0;
    double speed = 0;
    double steering = 0;
    double trackWidth = 10;
    double trackAngle = 0;
    double old_target_angle = 0;
    final double timeStep;

    boolean DEBUG_TEXT = false;

    public Benchmark(double timeStep) {
        this.timeStep = timeStep;
    }

    public void setNextTargetPosition(double target_pos) {
        this.old_old_target_pos = this.old_target_pos;
        this.old_target_pos = this.target_pos;
        this.target_pos = target_pos;
    }

    public void setSpeed(double speed) {
        this.speed = speed;
    }
    public void setSteering(double steering){
        this.steering = steering;
    }

    public void setTrackWidth(double trackWidth) {
        this.trackWidth = trackWidth;
    }

    public void setTrackAngle(double trackAngle) {
        this.trackAngle = trackAngle;
    }

    public void setCurrentPosition(double position) {
        double stepSize = Math.max(1,speed)*timeStep;   // step_size needs to be >= 0
        double quot = (old_target_pos-old_old_target_pos)*trackWidth/(2*stepSize);
        if(Math.abs(quot) >= 1)
        {
            if( DEBUG_TEXT )
                System.out.println("TargetPosition is not reachable in one step!");
        }
        else{
            double target_angle = Math.asin(quot);
            BenchmarkEntry e = new BenchmarkEntry(position, old_target_pos, speed, steering, target_angle-trackAngle);
            entries.add(e);

            old_target_angle = target_angle;
        }
    }

    public Statistic getStatistic() {
        return new Statistic(entries);
    }

    public void clear() {

        entries.clear();
        this.target_pos = 0;
        this.speed = 0;
    }
}
