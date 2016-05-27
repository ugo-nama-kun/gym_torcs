/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.experiment;

import de.delbrueg.steering.behaviour.CircleSteeringSettings;

/**
 *
 * @author Tim
 */
public enum Experiments {


    elchtest{
        @Override
        protected void setupExperiment(Experiment e) {
            e.plan = Plans.sinus_fast;
        }
    },
    elchtest_extrem{
        @Override
        protected void setupExperiment(Experiment e) {
            e.plan = Plans.sinus_very_fast;
        }
    },
    race{
        @Override
        protected void setupExperiment(Experiment e) {
            e.plan = Plans.complete;
        }
    },
    hold_position{
        @Override
        protected void setupExperiment(Experiment e) {
            e.plan = Plans.zero;
        }
    },
    random{
        @Override
        protected void setupExperiment(Experiment e) {
            e.plan = Plans.random;
        }
    };

    protected abstract void setupExperiment(Experiment e);

    public final Experiment create(){
        Experiment e = new Experiment();
        setupExperiment(e);
        return e;
    }
}
