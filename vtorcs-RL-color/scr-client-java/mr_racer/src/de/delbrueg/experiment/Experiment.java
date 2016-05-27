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
public class Experiment {
    public Plans plan;

    public String getContentHeadLine(String seperator){
        String content = "";
        content += plan.getContentHeadLine(seperator);
        content += seperator;
        content += CircleSteeringSettings.getContentHeadLine(seperator);

        return content;
    }
    public String getContentAsLine(String seperator){
        String content = "";
        content += plan.getContentAsLine(seperator);
        content += seperator;
        content += CircleSteeringSettings.getContentAsLine(seperator);

        return content;
    }
}
