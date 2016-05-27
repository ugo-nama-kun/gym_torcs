/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.lut;

import de.janquadflieg.mrracer.Utils;

import java.util.StringTokenizer;

/**
 *
 * @author Jan Quadflieg
 */
public class Dimension {
    private String name = "Noname";
    private double lb = 0.0;
    private double ub = 100.0;
    private int size = 10;    

    private Dimension(){
        
    }

    public Dimension(String s){
        StringTokenizer t = new StringTokenizer(s, " ");

        name = t.nextToken();
        lb = Double.parseDouble(t.nextToken());
        ub = Double.parseDouble(t.nextToken());
        size = Integer.parseInt(t.nextToken());
    }

    public Dimension(String name, double[] bounds, int size){
        if(bounds.length != 2){
            throw new RuntimeException();
        }
        if(size < 1){
            throw new RuntimeException();
        }
        if(bounds[1] <= bounds[0]){
            throw new RuntimeException();
        }

        this.name = name;
        this.lb = bounds[0];
        this.ub = bounds[1];
        this.size = size;
    }

    public Dimension(Dimension other){
        this.name = other.name;
        this.lb = other.lb;
        this.ub = other.ub;
        this.size = other.size;
    }

    public boolean contains(double d){
        if(d >= lb && d <= ub){
            return true;
        }

        return false;
    }

    public int getSize(){
        return size;
    }    

    public double getCenter(int index){
        if(index < 0 || index >= size){
            throw new RuntimeException();
        }

        double[] b = getBoundaries(index);

        return (b[0]+b[1]) / 2.0;
    }

    public double[] getBoundaries(int index){
        if(index < 0 || index >= size){
            throw new RuntimeException();
        }

        double range = ub -lb;
        double delta = range / (double) size;

        double[] result = new double[2];

        result[0] = lb + (index * delta);
        result[1] = lb + ((index+1) *delta);

        return result;
    }

    public int indexOf(double d){
        if(d < lb){
            return 0;

        } else if(d >= ub){
            return size-1;

        } else {
            double range = ub - lb;
            d -= lb;

            double i = Math.floor((d / range) * size);

            return (int)i;
        }
    }

    // only for backward compatibility
    public int oldIndexOf(double d){
        if(d < lb || d >= ub){
            return -1;

        } else {
            double range = ub - lb;
            d -= lb;

            double i = Math.floor((d / range) * size);

            return (int)i;
        }
    }

    public void write(java.io.OutputStreamWriter w)
            throws Exception {
        w.write(name);
        w.write(" ");
        w.write(String.valueOf(lb));
        w.write(" ");
        w.write(String.valueOf(ub));
        w.write(" ");
        w.write(String.valueOf(size));
    }

    public String toString(){
        return "Dimension \""+name+"\" ["+Utils.dTS(lb)+", "+Utils.dTS(ub)+
                "[ size="+size;
    }

    public static void main(String[] args){
        Dimension dim = new Dimension("Test", new double[]{-100, 150}, 15);

        System.out.println(dim);

        for(int i = 0; i < 2300; ++i){
            double d = -110 + (i*0.1);
            System.out.println(Utils.dTS(d)+", "+dim.oldIndexOf(d));
        }

        for(int i=0; i < dim.getSize(); ++i){
            double[] bounds = dim.getBoundaries(i);
            System.out.println("Bounds "+i+" ["+Utils.dTS(bounds[0])+
                    ", "+Utils.dTS(bounds[1])+"[");
        }
    }
}