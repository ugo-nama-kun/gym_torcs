/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.behaviour.decision;

/**
 *
 * @author Tim
 */
public class QuadraticFailureDecider extends LineOrCircleDecider {

    @Override
    public boolean isLinear() {
        return data.lineSit.getQuadraticFailure() <= data.circleSit.getQuadraticFailure();
    }

}
