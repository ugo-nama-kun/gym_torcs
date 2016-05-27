package de.delbrueg.steering.behaviour;

import de.delbrueg.experiment.OutputContentAsLine;
import de.janquadflieg.mrracer.behaviour.SteeringBehaviour;
import de.janquadflieg.mrracer.telemetry.ModifiableAction;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.TrackSegment;
import de.delbrueg.steering.benchmark.*;

public class BenchmarkSteeringBehaviour implements SteeringBehaviour, OutputContentAsLine{

    // the Behaviour to benchmark
    private SteeringBehaviour behaviour;

    private Benchmark benchmark = null;


    public BenchmarkSteeringBehaviour(SteeringBehaviour behaviour) {
        this.behaviour = behaviour;
        benchmark = new Benchmark(CircleSteeringSettings.time_step);
    }

    @Override
    public void setTargetPosition(java.awt.geom.Point2D position) {
        behaviour.setTargetPosition(position);
        benchmark.setNextTargetPosition(position.getX());
    }

    @Override
    public void setTrackSegment(TrackSegment s) {
        behaviour.setTrackSegment(s);
    }

    @Override
    public void setWidth(double width) {
        behaviour.setWidth(width);
        benchmark.setTrackWidth(width);
    }

    @Override
    public void reset() {
        behaviour.reset();
        benchmark.clear();
    }

    @Override
    public void execute(SensorData data, ModifiableAction action) {
        benchmark.setSpeed(data.getSpeed());
        benchmark.setCurrentPosition(data.getTrackPosition());
        benchmark.setTrackAngle(data.getAngleToTrackAxis());
        
        behaviour.execute(data, action);

        benchmark.setSteering(action.getSteering());
    }

    /**
     * The current situation of the car, as classified by an
     * appropriate classifier chosen by the controller which uses this behaviour.
     */
    @Override
    public void setSituation(de.janquadflieg.mrracer.classification.Situation s){

    }

    @Override
    public void shutdown() {
        behaviour.shutdown();

        Statistic stat = benchmark.getStatistic();
        stat.dump();
    }

    @Override
    public String getDebugInfo() {
        return behaviour.getDebugInfo();
    }

    @Override
    public String getContentHeadLine(String seperator) {
        return benchmark.getStatistic().getContentHeadLine(seperator);
    }

    @Override
    public String getContentAsLine(String seperator) {
        return benchmark.getStatistic().getContentAsLine(seperator);
    }

    @Override
    public void setParameters(java.util.Properties params, String prefix){
    }

    @Override
    public void getParameters(java.util.Properties params, String prefix){
    }
}