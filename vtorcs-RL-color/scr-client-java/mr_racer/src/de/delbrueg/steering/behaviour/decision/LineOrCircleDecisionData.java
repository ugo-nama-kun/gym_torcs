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
public final class LineOrCircleDecisionData{
    public final CircleSituation circleSit;
    public final LineSituation lineSit;
    public final double trackWidth;
    public final double trackPosition;

    public LineOrCircleDecisionData(CircleSituation circleSit, LineSituation lineSit, double trackWidth, double trackPosition) {
        this.circleSit = circleSit;
        this.lineSit = lineSit;
        this.trackWidth = trackWidth;
        this.trackPosition = trackPosition;
    }


}