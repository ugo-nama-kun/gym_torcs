package de.delbrueg.steering.behaviour;

import de.delbrueg.steering.behaviour.decision.LineOrCircleDeciders;
import de.delbrueg.predictor.lookahead_distance.LookAheadDistancePredictors;
import de.delbrueg.predictor.position.TargetPosPredictors;
import de.delbrueg.steering.wheelAngle.WheelAngleCalculators;

/**
 * This class combines all the different control parameters of the CircleSteeringBehaviour
 * @author Tim
 */
public class CircleSteeringSettings {

    /**
     * use plan target pos or stay in middle
     */
    public static boolean ignore_plan = false;

    /**
     * time between updates in seconds
     */
    public static double time_step = 0.19;

    /**
     * distance of front and rear axis
     */
    public static double car_track_width = 4.875;

    /**
     * weight applied to position adjustment
     * 0 = no position adjustment
     * 1 = try to be at target position at next time step
     */
    public static double target_position_weight = 1;

    /**
     * specifies the formula used to determine the distance to look ahead
     * in order to build circles from sensor data
     */
    public static LookAheadDistancePredictors lookahead_dist_predictor = LookAheadDistancePredictors.simplyTrackWidth;

    /**
     * specifies the used target position predictor
     */
    public static TargetPosPredictors target_pos_predictor_circles = TargetPosPredictors.CarPosCircle;

    /**
     * specifies the used target position predictor
     */
    public static TargetPosPredictors target_pos_predictor_lines = TargetPosPredictors.WeightedLineTargetPositionPredictor;

    /**
     * specifies the used wheel angle calculator
     */
    public static WheelAngleCalculators wheel_angle_calculator = WheelAngleCalculators.SimpleQuotient;

    /**
     * lines are perhaps more useful for use with other predictors that CarPosCircle
     */
    public static boolean use_lines = false;

    /**
     * the one that decides wether to use lines or circles
     */
    public static LineOrCircleDeciders line_or_circle_decider = LineOrCircleDeciders.QuadraticFailureDecider;

    /**
     * when calculating left/right circles, use only the side with more points and
     * get the other side by reducing the radius by trackWidth
     */
    public static boolean only_use_side_with_more_points = false;

    /**
     * make targetPositions more smooth
     */
    public static boolean smooth_target_pos = false;

    /**
     * dynamicly increase time_step if otherwise the
     */
    public static boolean timestep_dynamic = false;

    /**
     * (only used with timestep_dynamic)
     * 
     * size of dynamic part: 0.3 means x + [-0.3, 0.3]
     */
    public static double timestep_dynamic_interval_size = 0.3;

    /**
     * mass of car in kg (only used with extended quotient wheel angle calculator)
     *
     * mass of car1-trb1 out of xml file
     */
    public static double car_mass = 1150;

    /**
     * (only used with extended quotient wheel angle calculator)
     * 
     * negative = oversteering
     * zero = neutral
     * positive = understeering
     *
     * seems to be zero because all stiffness parameters are equal...
     */
    public static double car_prop_steering_factor = 0;

    // should not be used
    private CircleSteeringSettings() {
    }




    /**
     * write all settings into one line
     * @param seperator the seperator between different cells
     * @return String containing the line
     */
    public static String getContentAsLine(String seperator){
        String content = "";
        content += time_step;
        content += seperator;
        content += timestep_dynamic;
        content += seperator;
        content += timestep_dynamic_interval_size;
        content += seperator;
        content += car_track_width;
        content += seperator;
        content += smooth_target_pos;
        content += seperator;
        content += car_prop_steering_factor;
        content += seperator;
        content += use_lines;
        content += seperator;
        content += line_or_circle_decider;
        content += seperator;
        content += lookahead_dist_predictor;
        content += seperator;
        content += only_use_side_with_more_points;
        content += seperator;
        content += target_pos_predictor_circles;
        content += seperator;
        content += target_pos_predictor_lines;
        content += seperator;
        content += target_position_weight;
        content += seperator;
        content += wheel_angle_calculator;
        content += seperator;
        content += ignore_plan;
        content += seperator;
        content += car_mass;

        return content;
    }

    /**
     * write all settings into one line
     * @param seperator the seperator between different cells
     * @return String containing the line
     */
    public static String getContentHeadLine(String seperator){
        String content = "";
        content += "time_step";
        content += seperator;
        content += "timestep_dynamic";
        content += seperator;
        content += "timestep_dynamic_interval_size";
        content += seperator;
        content += "car_track_width";
        content += seperator;
        content += "smooth_target_pos";
        content += seperator;
        content += "car_prop_steering_factor";
        content += seperator;
        content += "use_lines";
        content += seperator;
        content += "line_or_circle_decider";
        content += seperator;
        content += "lookahead_dist_predictor";
        content += seperator;
        content += "only_use_side_with_more_points";
        content += seperator;
        content += "target_pos_predictor_circles";
        content += seperator;
        content += "target_pos_predictor_lines";
        content += seperator;
        content += "target_position_weight";
        content += seperator;
        content += "wheel_angle_calculator";
        content += seperator;
        content += "ignore_plan";
        content += seperator;
        content += "car_mass";

        return content;
    }
}
