/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.behaviour.decision;

import de.delbrueg.steering.classification.CircleSituation;
import de.delbrueg.steering.classification.LineSituation;

/**
 *
 * @author Tim
 */
public abstract class LineOrCircleDecider {

    protected LineOrCircleDecisionData data;

    public final void setData(LineOrCircleDecisionData data) {
        this.data = data;
    }

    /**
     * this is the main decision function. setData has to be called before!
     * @return
     */
    public abstract boolean isLinear();
}
