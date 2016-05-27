/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.benchmark;

import de.delbrueg.experiment.OutputContentAsLine;
import java.util.List;

/**
 *
 * @author Tim
 */
public class Statistic implements OutputContentAsLine{

    int numEntries = 0;
    double squaredDiffSum = 0;
    double diffSum = 0;
    double squaredSteerSum = 0;
    double maxDiff = 0;
    double squaredAngleFailureSum = 0;

    protected Statistic( List<BenchmarkEntry> list){

        numEntries = list.size();
        
        for(BenchmarkEntry e: list){

            double diff = Math.abs(e.position-e.target_position);

            diffSum += diff;
            squaredDiffSum += Math.pow(diff,2);
            squaredSteerSum += Math.pow(1+Math.abs(e.steering),2);
            squaredAngleFailureSum += Math.pow(e.target_angle_diff, 2);
            maxDiff = Math.max(maxDiff, diff);
        }
    }

    public int getNumEntries() {
        return numEntries;
    }

    public double getDiffSum() {
        return diffSum;
    }

    public double getMaxDiff() {
        return maxDiff;
    }

    public double getSquaredDiffSum() {
        return squaredDiffSum;
    }

    public double getSquaredDiffTimesSteeringSum() {
        return squaredSteerSum;
    }

    public double getNormalizedDiffSum() {
        return diffSum / numEntries;
    }

    public double getNormalizedSquaredDiffSum() {
        return squaredDiffSum / numEntries;
    }

    public double getNormalizedSquaredSteeringSum() {
        return squaredSteerSum / numEntries;
    }

    public void dump(){
        System.out.println("----------------------");
        System.out.println("Statistic: ");
        System.out.println("----------------------");

        System.out.println("numEntries: " + numEntries);
        System.out.println("maxDiff: " + maxDiff);
        System.out.println("diffSum: " + diffSum);
        System.out.println("squaredDiffSum: " + squaredDiffSum);
        System.out.println("squaredDiffSumWeighted: " + squaredDiffSum/numEntries);
        System.out.println("squaredSteerSum: " + squaredSteerSum);
        System.out.println("squaredSteerSumWeighted: " + squaredSteerSum/numEntries);

        System.out.println("----------------------");
    }

    @Override
    public String getContentHeadLine(String seperator) {
        String content = "";
        
        content += "numEntries";
        content += seperator;
        content += "maxDiff";
        content += seperator;
        content += "squaredDiffSumWeighted";
        content += seperator;
        content += "squaredSteerSumWeighted";
        content += seperator;
        content += "squaredAngleFailureSumWeighted";

        return content;
    }

    @Override
    public String getContentAsLine(String seperator) {
        String content = "";

        content += numEntries;
        content += seperator;
        content += maxDiff;
        content += seperator;
        content += squaredDiffSum/numEntries;
        content += seperator;
        content += squaredSteerSum/numEntries;
        content += seperator;
        content += squaredAngleFailureSum/numEntries;

        return content;
    }

    
}
