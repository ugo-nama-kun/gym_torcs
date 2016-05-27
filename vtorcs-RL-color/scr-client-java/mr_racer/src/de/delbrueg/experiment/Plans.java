/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.experiment;

import de.delbrueg.steering.behaviour.CircleSteeringSettings;
import de.janquadflieg.mrracer.opponents.DummyObserver;
import de.janquadflieg.mrracer.plan.Plan2011;

/**
 *
 * @author Tim
 */
public enum Plans implements OutputContentAsLine{
    complete{
        @Override
        public PlanInterface create(){
            return new Plan2011(DEFAULT_PATH, true, new DummyObserver(), null);
        }
    },
    sinus_fast{
        @Override
        public PlanInterface create(){
            double period = 1/40.0;
            double speed = 250;
            return new SinusPlan(period, speed, 0.7);
        }
    },
    sinus_very_fast{
        @Override
        public PlanInterface create(){
            double period = 1/25.0;
            double speed = 250;
            return new SinusPlan(period, speed, 0.7);
        }
    },
    sinus_slow{
        @Override
        public PlanInterface create(){
            double period = 1/5.0;
            double speed = 70;
            return new SinusPlan(period, speed, 0.8);
        }
    },
    zero{
        @Override
        public PlanInterface create(){
            double speed = 80;
            double target_pos = 0;
            double max_adaption_step = 2.0;
            return new ConstantPlan(speed,target_pos,max_adaption_step);
        }
    },
    random{
        @Override
        public PlanInterface create(){
            return new RandomPlan(0.02, 80, 0.3, CircleSteeringSettings.time_step);
        }
    };

    private static final String DEFAULT_PATH = "/de/janquadflieg/mrracer/data/cigtables/";
    public abstract PlanInterface create();

    @Override
    public String getContentAsLine(String seperator){
        return this.toString();
    }

    @Override
    public String getContentHeadLine(String seperator){
        return "Plan Name";
    }
}
