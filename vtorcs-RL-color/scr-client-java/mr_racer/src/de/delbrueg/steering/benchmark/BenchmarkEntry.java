/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.benchmark;

/**
 *
 * @author Tim
 */
class BenchmarkEntry {
    protected final double position;
    protected final double target_position;
    protected final double speed;
    protected final double steering;
    protected final double target_angle_diff;

    public BenchmarkEntry(double position, double target_position, double speed, double steering, double target_angle_diff) {
        this.position = position;
        this.target_position = target_position;
        this.speed = speed;
        this.steering = steering;
        this.target_angle_diff = target_angle_diff;
    }
}
