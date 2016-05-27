package scr;

/**
 * Created by IntelliJ IDEA.
 * User: Administrator
 * Date: Mar 4, 2008
 * Time: 12:18:47 PM
 */
public interface SensorModel {

    // basic information about your car and the track (you probably should take care of these somehow)

    public double getSpeed ();

    public double getAngleToTrackAxis ();

    public double[] getTrackEdgeSensors ();
    
    public double[] getFocusSensors ();//ML

    public double getTrackPosition();

    public int getGear ();

    // basic information about other cars (only useful for multi-car races)

    public double[] getOpponentSensors ();

    public int getRacePosition ();

    // additional information (use if you need)

    public double getLateralSpeed ();
    

    public double getCurrentLapTime ();

    public double getDamage ();

    public double getDistanceFromStartLine ();

    public double getDistanceRaced ();

    public double getFuelLevel ();

    public double getLastLapTime ();

    public double getRPM ();

    public double[] getWheelSpinVelocity ();
    
    public double getZSpeed ();
    
    public double getZ ();
    
    public String getMessage();
    
}
