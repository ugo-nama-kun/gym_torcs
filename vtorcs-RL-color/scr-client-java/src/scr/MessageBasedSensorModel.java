package scr;


/**
 * Created by IntelliJ IDEA.
 * User: Administrator
 * Date: Mar 4, 2008
 * Time: 3:44:29 PM
 */
public class MessageBasedSensorModel implements SensorModel {

    private MessageParser message;

    public MessageBasedSensorModel (MessageParser message) {
        this.message = message;
    }
    
    public MessageBasedSensorModel (String strMessage) {
        this.message = new MessageParser(strMessage);
    }

    public double getSpeed() {
        return (Double) message.getReading("speedX");
    }

    public double getAngleToTrackAxis() {
        return (Double) message.getReading("angle");
    }

    public double[] getTrackEdgeSensors() {
        return (double[]) message.getReading("track");
    }

    public double[] getFocusSensors() {//ML
    	return (double[]) message.getReading("focus");//ML
    }//ML

    public int getGear() {
        // This really can't be how they wanted it to be when they came up with generics!
        return (int) (double) (Double) message.getReading("gear");
    }

    public double[] getOpponentSensors() {
        return (double[]) message.getReading("opponents");
    }

    public int getRacePosition() {
        return (int) (double) (Double) message.getReading("racePos");
    }

    public double getLateralSpeed() {
        return (Double) message.getReading("speedY");
    }

    public double getCurrentLapTime() {
        return (Double) message.getReading("curLapTime");
    }

    public double getDamage() {
        return (Double) message.getReading("damage");
    }

    public double getDistanceFromStartLine() {
        return (Double) message.getReading("distFromStart");
    }

    public double getDistanceRaced() {
        return (Double) message.getReading("distRaced");
    }

    public double getFuelLevel() {
        return (Double) message.getReading("fuel");
    }

    public double getLastLapTime() {
        return (Double) message.getReading("lastLapTime");
    }

    public double getRPM() {
        return (Double) message.getReading("rpm");
    }

    public double getTrackPosition() {
        return (Double) message.getReading("trackPos");
    }

    public double[] getWheelSpinVelocity() {
        return (double[]) message.getReading("wheelSpinVel");
    }

	public String getMessage() {
		return message.getMessage();
	}

	public double getZ() {
		return (Double) message.getReading("z"); 
	}

	public double getZSpeed() {
		return (Double) message.getReading("speedZ");
	}

}
