package scr;


/**
 * Created by IntelliJ IDEA.
 * User: Administrator
 * Date: Mar 4, 2008
 * Time: 4:59:21 PM

 */
public class DeadSimpleSoloController extends Controller {

    final double targetSpeed = 15;


    public Action control(SensorModel sensorModel) {
        Action action = new Action ();
        if (sensorModel.getSpeed () < targetSpeed) {
            action.accelerate = 1;
        }
        if (sensorModel.getAngleToTrackAxis() < 0) {
            action.steering = -0.1;
        }
        else {
            action.steering = 0.1;
        }
        action.gear = 1;
        return action;
    }

    public void reset() {
		System.out.println("Restarting the race!");
		
	}

	public void shutdown() {
		System.out.println("Bye bye!");		
	}
}
