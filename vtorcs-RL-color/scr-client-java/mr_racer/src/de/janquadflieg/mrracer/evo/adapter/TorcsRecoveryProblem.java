/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.evo.adapter;

import de.janquadflieg.mrracer.evo.FitnessEvaluator;
import de.janquadflieg.mrracer.behaviour.DefensiveFallbackBehaviour;
import de.janquadflieg.mrracer.behaviour.OffTrackRecoveryBehaviour;
import de.janquadflieg.mrracer.controller.CrashController;
import de.janquadflieg.mrracer.controller.Evaluator;
import de.janquadflieg.mrracer.controller.MrRacer2011;
import de.janquadflieg.mrracer.functions.GeneralisedLogisticFunction;
import de.janquadflieg.mrracer.plan.Plan2011;

import java.util.Properties;

import optimizer.ea.*;

/**
 * Interface betweeen Mike's ea lib and Torcs. This one is used to optimize
 * the recovery behaviours.
 *
 * @author quad
 */
public class TorcsRecoveryProblem
extends Problem implements ParallelEvaluableProblem{

    private static final String TORCS_IP_PORT = "Torcs.IP_PORT";

    private static final String TORCS_MAX_TICKS = "Torcs.MAX_TICKS";

    private static final String TORCS_MAX_LAPS = "Torcs.MAX_LAPS";

    private static final String TORCS_TRACK = "Torcs.TRACK";
    
    private int port = 3001;

    private String host = "127.0.0.1";

    private String trackName = "ea_run_noisy_wheel2";
    
    /** Dimensions. */
    private int dimensions = 6;

    private static final int DEFAULT_TICKS = 10000;    

    private int maxTicks = DEFAULT_TICKS;
    private int maxLaps = de.janquadflieg.mrracer.controller.Evaluator.NO_MAXIMUM;
    

    /** Creates a new instance of Torcs Problem. */
    public TorcsRecoveryProblem(Parameters par) {

        if (par != null) {
            if(par.origConfig.containsKey(TORCS_IP_PORT)){
                String ipport = par.origConfig.getProperty(TORCS_IP_PORT);
                int idx = ipport.indexOf(':');
                host = ipport.substring(0, idx);
                port = Integer.parseInt(ipport.substring(idx+1, ipport.length()));
            }
            if(par.origConfig.containsKey(TORCS_MAX_TICKS)){
                maxTicks = Integer.parseInt(par.origConfig.getProperty(TORCS_MAX_TICKS));
            }
            if(par.origConfig.containsKey(TORCS_MAX_LAPS)){
                maxLaps = Integer.parseInt(par.origConfig.getProperty(TORCS_MAX_LAPS));
            }
            if(par.origConfig.containsKey(TORCS_TRACK)){
                trackName = par.origConfig.getProperty(TORCS_TRACK).trim();
            }
        } 
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
    public void preParallelEvaluations(int n){
        // nothing to do here
    }

    /**
     * Method called after the parallel evaluation of a number of individuals
     * has finished. Can be used to clean up the stuff generated during the call
     * to preParallelEvaluation().
     */
    public void postParallelEvaluations(){
        // nothing to do here
    }

    /**
     * Method called to start the evaluation of one individual.
     * @param ind The individual to evaluate.
     * @param i The index of the individual, a number between 1 and n.
     * @return The results of the evaluation.
     */
    public Evaluation parallelEvaluation(Individual ind, int i){
        return evaluate(ind, i-1);
    }

    public Evaluation evaluate(Individual variables) {
        return evaluate(variables, 0);
    }

    public Evaluation evaluate(Individual variables, int offset) {
        Properties params = new Properties();

        RealVariable v;

        // offtrack forward        
        v = (RealVariable)variables.getAnyVariable("OFF_F_ANGLE");
        params.setProperty(MrRacer2011.RECOVERY+DefensiveFallbackBehaviour.OFF_TRACK+OffTrackRecoveryBehaviour.F_ANGLE,
                String.valueOf(v.toDouble()*90.0));
        v = (RealVariable)variables.getAnyVariable("OFF_F_MIN");
        params.setProperty(MrRacer2011.RECOVERY+DefensiveFallbackBehaviour.OFF_TRACK+OffTrackRecoveryBehaviour.F_MIN_ACC,
                String.valueOf(v.toDouble()));
        v = (RealVariable)variables.getAnyVariable("OFF_F_MAX");
        params.setProperty(MrRacer2011.RECOVERY+DefensiveFallbackBehaviour.OFF_TRACK+OffTrackRecoveryBehaviour.F_MAX_ACC,
                String.valueOf(v.toDouble()));


        // offtrack backward
        v = (RealVariable)variables.getAnyVariable("OFF_B_ANGLE");
        params.setProperty(MrRacer2011.RECOVERY+DefensiveFallbackBehaviour.OFF_TRACK+OffTrackRecoveryBehaviour.B_ANGLE,
                String.valueOf(v.toDouble()*-90.0));
        v = (RealVariable)variables.getAnyVariable("OFF_B_MIN");
        params.setProperty(MrRacer2011.RECOVERY+DefensiveFallbackBehaviour.OFF_TRACK+OffTrackRecoveryBehaviour.B_MIN_ACC,
                String.valueOf(v.toDouble()));
        v = (RealVariable)variables.getAnyVariable("OFF_B_MAX");
        params.setProperty(MrRacer2011.RECOVERY+DefensiveFallbackBehaviour.OFF_TRACK+OffTrackRecoveryBehaviour.B_MAX_ACC,
                String.valueOf(v.toDouble()));        
        
        // wait a bit
        try {
            Thread.sleep(1000);
        } catch (Exception e) {
        }        

        MrRacer2011 controller = new MrRacer2011();

        
        controller.setParameters(params);
        controller.setStage(champ2011client.Controller.Stage.QUALIFYING);
        controller.setTrackName(trackName);

        CrashController crashController = new CrashController(controller);
        crashController.setStage(champ2011client.Controller.Stage.QUALIFYING);
        crashController.setTrackName(trackName);        

        FitnessEvaluator fe = new FitnessEvaluator(host, port+offset,
                new Evaluator(crashController, maxTicks, true), maxLaps);

        while (!fe.finished()) {
            try {
                Thread.sleep(1000);
            } catch (Exception e) {
            }
        }

        synchronized(this){
            ++evals;
            ++validEvals;
        }

        variables.evaluated = true;

        double fitness = fe.getOverallTime();
        boolean valid = !fe.maxDamageReached();

        System.out.println("Fitness: "+fitness);

        //return new Evaluation(-fe.getResult().getFitness(), true);
        //return new Evaluation(fe.getFastestLap(), true);
        return new Evaluation(fitness, valid);
    }

    public Individual getTemplate(Parameters par) {
        Individual result = new Individual();

        VariableGroup genotype = new VariableGroup("genotype");
        // offtrack forward
        genotype.add(new RealVariable("OFF_F_ANGLE", 0, 1, true));
        genotype.add(new RealVariable("OFF_F_MIN", 0, 1, true));
        genotype.add(new RealVariable("OFF_F_MAX", 0, 1, true));
        // offtrack backward
        genotype.add(new RealVariable("OFF_B_ANGLE", 0, 1, true));
        genotype.add(new RealVariable("OFF_B_MIN", 0, 1, true));
        genotype.add(new RealVariable("OFF_B_MAX", 0, 1, true));
        

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

        BlackBoxProblem prob = new TorcsRecoveryProblem(par);
        Individual ind = prob.getTemplate(par);
        BatchEA ea = new BatchEA(ind, par, prob);
        ea.run();
    }
}
