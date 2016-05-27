/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.evo.adapter;

import de.janquadflieg.mrracer.evo.FitnessEvaluator;
import de.janquadflieg.mrracer.behaviour.DampedAccelerationBehaviour;
//import de.janquadflieg.mrracer.controller.MrRacer2010;
import de.janquadflieg.mrracer.controller.Evaluator;
import de.janquadflieg.mrracer.controller.MrRacer2011;
import de.janquadflieg.mrracer.functions.GeneralisedLogisticFunction;
import de.janquadflieg.mrracer.plan.Plan2011;

import java.io.*;
import java.util.*;

import optimizer.ea.*;

/**
 * Interface betweeen Mike's ea lib and Torcs.
 *
 * @author quad
 */
public class TorcsProblem
        extends Problem implements ParallelEvaluableProblem {

    private static final String TORCS_IP_PORT = "Torcs.IP_PORT";
    private static final String TORCS_MAX_TICKS = "Torcs.MAX_TICKS";
    private static final String TORCS_MAX_LAPS = "Torcs.MAX_LAPS";
    private static final String TORCS_TRACK = "Torcs.TRACK";
    private int port = 3001;
    private String host = "127.0.0.1";
    private String trackName = "ea_run_noisy_wheel2";
    private String paramFile = "ea_run.params";
    private Properties defaultProperties = new Properties();
    /** Dimensions. */
    private int dimensions = 13;
    private static final int DEFAULT_TICKS = 10000;
    private int maxTicks = DEFAULT_TICKS;
    private int maxLaps = de.janquadflieg.mrracer.controller.Evaluator.NO_MAXIMUM;

    /** Ip port for function evaluations. */
    /** Creates a new instance of Torcs Problem. */
    public TorcsProblem(Parameters par) {

        System.setProperty("EAMode", "");

        if (par != null) {
            if (par.origConfig.containsKey(TORCS_IP_PORT)) {
                String ipport = par.origConfig.getProperty(TORCS_IP_PORT);
                int idx = ipport.indexOf(':');
                host = ipport.substring(0, idx);
                port = Integer.parseInt(ipport.substring(idx + 1, ipport.length()));
            }
            if (par.origConfig.containsKey(TORCS_MAX_TICKS)) {
                maxTicks = Integer.parseInt(par.origConfig.getProperty(TORCS_MAX_TICKS));
            }
            if (par.origConfig.containsKey(TORCS_MAX_LAPS)) {
                maxLaps = Integer.parseInt(par.origConfig.getProperty(TORCS_MAX_LAPS));
            }
            if (par.origConfig.containsKey(TORCS_TRACK)) {
                trackName = par.origConfig.getProperty(TORCS_TRACK).trim();
            }
        }

        if (new File(paramFile).exists()) {
            //System.out.print("Loading default parameter set ");

            try {
                InputStream in = new FileInputStream(paramFile);

                defaultProperties.load(in);
                in.close();

            } catch (Exception e) {
                e.printStackTrace(System.out);
            }
        }
        
    }

    @Override
    public String problemRunHeader() {
        Iterator<String> it = defaultProperties.stringPropertyNames().iterator();
        StringBuilder result = new StringBuilder();

        while(it.hasNext()){
            String key = it.next();

            result.append(key).append("=").append(defaultProperties.getProperty(key)).append("\n");
        }
        
        return result.toString();
    }

    /**
     * A method called before the next round of parallel evaluations start. This
     * can be used to setup some metadata used to evaluate the n individuals
     * (determine port numbers for network connections of the individuals, prepare
     * different temp directories for the individuals, etc).
     *
     * @param n Number of individuals to be evaluated in parallel
     * during the next batch.
     */
    public void preParallelEvaluations(int n) {
        // nothing to do here
    }

    /**
     * Method called after the parallel evaluation of a number of individuals
     * has finished. Can be used to clean up the stuff generated during the call
     * to preParallelEvaluation().
     */
    public void postParallelEvaluations() {
        // nothing to do here
    }

    /**
     * Method called to start the evaluation of one individual.
     * @param ind The individual to evaluate.
     * @param i The index of the individual, a number between 1 and n.
     * @return The results of the evaluation.
     */
    public Evaluation parallelEvaluation(Individual ind, int i) {
        return evaluate(ind, i - 1);
    }

    public Evaluation evaluate(Individual variables) {
        return evaluate(variables, 0);
    }

    public Evaluation evaluate(Individual variables, int offset) {
        Properties params = new Properties();

        RealVariable v;

        // parameters for the target speed function
        v = (RealVariable) variables.getAnyVariable("TS_B");
        params.setProperty(MrRacer2011.PLAN + Plan2011.TARGET_SPEEDS + GeneralisedLogisticFunction.GROWTH_RATE_B,
                String.valueOf(Math.pow(10, v.toDouble())));

        v = (RealVariable) variables.getAnyVariable("TS_M");
        params.setProperty(MrRacer2011.PLAN + Plan2011.TARGET_SPEEDS + GeneralisedLogisticFunction.M,
                String.valueOf(v.toDouble()));

        v = (RealVariable) variables.getAnyVariable("TS_V");
        params.setProperty(MrRacer2011.PLAN + Plan2011.TARGET_SPEEDS + GeneralisedLogisticFunction.V,
                String.valueOf(v.toDouble()));

        v = (RealVariable) variables.getAnyVariable("TS_Q");
        params.setProperty(MrRacer2011.PLAN + Plan2011.TARGET_SPEEDS + GeneralisedLogisticFunction.Q,
                String.valueOf(v.toDouble()));

        // parameters for the acceleration damp function
        v = (RealVariable) variables.getAnyVariable("AD_B");
        params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.ACC_DAMP + GeneralisedLogisticFunction.GROWTH_RATE_B,
                String.valueOf(Math.pow(10, v.toDouble())));

        v = (RealVariable) variables.getAnyVariable("AD_M");
        params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.ACC_DAMP + GeneralisedLogisticFunction.M,
                String.valueOf(v.toDouble()));

        v = (RealVariable) variables.getAnyVariable("AD_V");
        params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.ACC_DAMP + GeneralisedLogisticFunction.V,
                String.valueOf(v.toDouble()));

        v = (RealVariable) variables.getAnyVariable("AD_Q");
        params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.ACC_DAMP + GeneralisedLogisticFunction.Q,
                String.valueOf(v.toDouble()));

        // parameters for the brake damp function
        v = (RealVariable) variables.getAnyVariable("BD_B");
        params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.BRAKE_DAMP + GeneralisedLogisticFunction.GROWTH_RATE_B,
                String.valueOf(Math.pow(10, v.toDouble())));

        v = (RealVariable) variables.getAnyVariable("BD_M");
        params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.BRAKE_DAMP + GeneralisedLogisticFunction.M,
                String.valueOf(v.toDouble()));

        v = (RealVariable) variables.getAnyVariable("BD_V");
        params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.BRAKE_DAMP + GeneralisedLogisticFunction.V,
                String.valueOf(v.toDouble()));

        v = (RealVariable) variables.getAnyVariable("BD_Q");
        params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.BRAKE_DAMP + GeneralisedLogisticFunction.Q,
                String.valueOf(v.toDouble()));

        // parameter for the brake coefficient in corners
        v = (RealVariable) variables.getAnyVariable("P_BCC");
        params.setProperty(MrRacer2011.PLAN + Plan2011.BRAKE_CORNER_COEFF,
                String.valueOf(v.toDouble()));

        // damp on straights?-MrRacer2011.Acc--DAB.dos-=true
        params.setProperty(MrRacer2011.ACC + DampedAccelerationBehaviour.ACC_DAMP_ON_STRAIGHT, String.valueOf(false));

        // wait a bit
        try {
            Thread.sleep(1000);
        } catch (Exception e) {
        }

        MrRacer2011 controller = new MrRacer2011();

        controller.setParameters(defaultProperties);
        controller.setParameters(params);
        controller.setStage(champ2011client.Controller.Stage.QUALIFYING);
        controller.setTrackName(trackName);

        FitnessEvaluator fe = new FitnessEvaluator(host, port + offset,
                new Evaluator(controller, maxTicks, true), maxLaps);

        while (!fe.finished()) {
            try {
                Thread.sleep(1000);
            } catch (Exception e) {
            }
        }

        synchronized (this) {
            ++evals;
            ++validEvals;
        }

        variables.evaluated = true;

        //return new Evaluation(-fe.getResult().getFitness(), true);
        //return new Evaluation(fe.getFastestLap(), true);
        return new Evaluation(fe.getOverallTime(), true);
    }

    public Individual getTemplate(Parameters par) {
        Individual result = new Individual();

        VariableGroup genotype = new VariableGroup("genotype");
        // growth rate b of the target speed
        genotype.add(new RealVariable("TS_B", 0, 1, true));
        // m of the target speed
        genotype.add(new RealVariable("TS_M", 0, 1, true));
        // v of the target speed
        genotype.add(new RealVariable("TS_V", 0, 1, true));
        // q of the target speed
        genotype.add(new RealVariable("TS_Q", 0, 1, true));

        // growth rate b of the acc damp
        genotype.add(new RealVariable("AD_B", 0, 1, true));
        // m of the acc damp
        genotype.add(new RealVariable("AD_M", 0, 1, true));
        // v of the acc damp
        genotype.add(new RealVariable("AD_V", 0, 1, true));
        // q of the acc damp
        genotype.add(new RealVariable("AD_Q", 0, 1, true));

        // growth rate b of the brake damp
        genotype.add(new RealVariable("BD_B", 0, 1, true));
        // m of the brake damp
        genotype.add(new RealVariable("BD_M", 0, 1, true));
        // v of the brake damp
        genotype.add(new RealVariable("BD_V", 0, 1, true));
        // q of the brake damp
        genotype.add(new RealVariable("BD_Q", 0, 1, true));

        // brake corner coefficient
        genotype.add(new RealVariable("P_BCC", 0, 1, true));

        result.importGroup(genotype);

        // one mutation strength:
        RealMetaVariable mutationStrength = new RealMetaVariable("mutationStrength", result.variables,
                Operators.CMUTATION_STRENGTH, par.minSigma, par.maxSigma);
        if (par.initSigmas > 0) {
            mutationStrength.setInitGaussian(par.initSigmas, par.initSigmaRange);
        }
        result.importMetadata(mutationStrength);


        return result;
    }

    public int dimensions() {
        return dimensions;
    }

    public static void main(String[] args) {
        //String paramName = "F:\\Quad\\Experiments\\Torcs-Test\\torcs-config.properties";
        String paramName = null;
        if (args.length == 1) {
            paramName = args[ 0];
        }
        Parameters par = new Parameters(args, paramName);

        BlackBoxProblem prob = new TorcsProblem(par);
        Individual ind = prob.getTemplate(par);
        BatchEA ea = new BatchEA(ind, par, prob);
        ea.run();
    }
}
