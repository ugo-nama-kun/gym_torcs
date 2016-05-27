/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.evo.tools;

import java.util.*;
import java.text.*;

import de.janquadflieg.mrracer.behaviour.*;
import de.janquadflieg.mrracer.controller.*;
import de.janquadflieg.mrracer.evo.Individual2011;
import de.janquadflieg.mrracer.functions.GeneralisedLogisticFunction;
import de.janquadflieg.mrracer.plan.Plan2011;

/**
 *
 * @author quad
 */
public class EvostarParser 
implements IndividualParser{

    public Individual2011 parse(String line, String track) {
        Individual2011 result = new Individual2011();
        result.properties = new Properties();

        Properties params = result.properties;

        DecimalFormat nf = new DecimalFormat();
        DecimalFormatSymbols symbols = nf.getDecimalFormatSymbols();
        symbols.setDecimalSeparator(',');
        symbols.setGroupingSeparator('.');
        nf.setDecimalFormatSymbols(symbols);

        StringTokenizer tokenizer = new StringTokenizer(line, " ");

        // number
        String token = tokenizer.nextToken();
        result.indNR = Integer.parseInt(token);
        result.indID = result.indNR + 1;
        // generation no
        tokenizer.nextToken();
        // fitness value
        String sfitness = tokenizer.nextToken();
        try {
            result.fitness.put(track, nf.parse(sfitness).doubleValue());

        } catch (ParseException e) {
            e.printStackTrace(System.out);
        }
        // mutation strength
        tokenizer.nextToken();

        try {
            // parameters for the planning module
            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.PLAN + Plan2011.TARGET_SPEEDS + GeneralisedLogisticFunction.GROWTH_RATE_B,
                    String.valueOf(Math.pow(10, nf.parse(token).doubleValue())));
            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.PLAN + Plan2011.TARGET_SPEEDS + GeneralisedLogisticFunction.M,
                    String.valueOf(nf.parse(token).doubleValue()));
            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.PLAN + Plan2011.TARGET_SPEEDS + GeneralisedLogisticFunction.V,
                    String.valueOf(nf.parse(token).doubleValue()));
            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.PLAN + Plan2011.TARGET_SPEEDS + GeneralisedLogisticFunction.Q,
                    String.valueOf(nf.parse(token).doubleValue()));

            // parameters for the acceleration damp function
            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.ACC_DAMP + GeneralisedLogisticFunction.GROWTH_RATE_B,
                    String.valueOf(Math.pow(10, nf.parse(token).doubleValue())));
            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.ACC_DAMP + GeneralisedLogisticFunction.M,
                    String.valueOf(nf.parse(token).doubleValue()));
            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.ACC_DAMP + GeneralisedLogisticFunction.V,
                    String.valueOf(nf.parse(token).doubleValue()));
            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.ACC_DAMP + GeneralisedLogisticFunction.Q,
                    String.valueOf(nf.parse(token).doubleValue()));

            // parameters for the brake damp function
            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.BRAKE_DAMP + GeneralisedLogisticFunction.GROWTH_RATE_B,
                    String.valueOf(Math.pow(10, nf.parse(token).doubleValue())));

            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.BRAKE_DAMP + GeneralisedLogisticFunction.M,
                    String.valueOf(nf.parse(token).doubleValue()));

            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.BRAKE_DAMP + GeneralisedLogisticFunction.V,
                    String.valueOf(nf.parse(token).doubleValue()));

            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.BRAKE_DAMP + GeneralisedLogisticFunction.Q,
                    String.valueOf(nf.parse(token).doubleValue()));

            token = tokenizer.nextToken();
            params.setProperty(MrRacer2011.PLAN + Plan2011.BRAKE_CORNER_COEFF,
                    String.valueOf(nf.parse(token).doubleValue()));

        } catch (Exception e) {
            e.printStackTrace(System.out);
        }

        return result;
    }

}
