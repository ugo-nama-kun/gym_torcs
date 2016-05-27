/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.lut;

import de.janquadflieg.mrracer.Utils;

import java.awt.*;
import java.io.*;
import java.util.*;

/**
 * A d-dimensional lookup table.
 *
 * @author Jan Quadflieg
 */
public class LookUpTable {

    private double[] values;
    private int[] readCtr;
    private int[] writeCtr;
    private Dimension[] dimensions;
    private int[] offsets;

    private LookUpTable() {
    }

    public LookUpTable(Dimension[] dims) {
        this.dimensions = new Dimension[dims.length];
        this.offsets = new int[dims.length];

        int size = 1;

        for (int i = 0; i < dims.length; ++i) {
            dimensions[i] = new Dimension(dims[i]);
            size *= dims[i].getSize();
        }

        //System.out.println("Size = "+size);
        values = new double[size];
        readCtr = new int[size];
        writeCtr = new int[size];

        offsets[dimensions.length - 1] = 1;
        for (int i = dimensions.length - 2; i >= 0; --i) {
            offsets[i] = dimensions[i + 1].getSize() * offsets[i + 1];
        }
        //for(int i=0; i < offsets.length; ++i){
        //  System.out.println("Offsets["+i+"] "+offsets[i]);
        //}

        reset();
    }

    public boolean contains(double[] d) {
        return oldIndexOf(d) != -1;
    }

    public double get(int i) {
        ++readCtr[i];
        return values[i];
    }    

    public double get(double[] d) {
        int index = oldIndexOf(d);

        if (index != -1) {
            ++readCtr[index];

            return values[index];
        }

        return Double.NaN;
    }

    public double getLinInt(double[] d) {
        return getLinInt(d, indicesOf(d), 0);
    }

    private double getLinInt(double[] d, int[] indices, int dim) {
        double center = dimensions[dim].getCenter(indices[dim]);
        double[] bounds = dimensions[dim].getBoundaries(indices[dim]);

        // in this case we cannot interpolate
        if((indices[dim] == 0 && d[dim] <= center) ||
                (indices[dim] == dimensions[dim].getSize()-1 && d[dim] >= center)){
            // check the depth
            if (dim + 1 == dimensions.length) {
               return values[indexOf(indices)];

            } else {
                return getLinInt(d, indices, dim + 1);
            }
        }

        int[] lower = Arrays.copyOf(indices, indices.length);
        int[] upper = Arrays.copyOf(indices, indices.length);
        double wl, wu;

        if (d[dim] <= center) {
            lower[dim] = Math.max(0, lower[dim] - 1);
            wu = 0.5 + (((d[dim] - bounds[0]) / (center - bounds[0])) * 0.5);
            wl = 1 - wu;

        } else {
            upper[dim] = Math.min(dimensions[dim].getSize() - 1, upper[dim] + 1);
            wl = 1 - (((d[dim] - center) / (bounds[1] - center)) * 0.5);
            wu = 1 - wl;
        }

        if (dim + 1 == dimensions.length) {
            return (values[indexOf(lower)] * wl) + (values[indexOf(upper)] * wu);

        } else {
            return (getLinInt(d, lower, dim + 1) * wl) + (getLinInt(d, upper, dim + 1) * wu);
        }
    }

    private int indexOf(int[] indices) {
        int result = 0;

        for (int i = 0; i < indices.length; ++i) {
            result += (indices[i] * offsets[i]);
        }

        return result;
    }

    public int indexOf(double[] d) {
        return indexOf(indicesOf(d));
    }

    private int[] indicesOf(double[] d) {
        if (d.length != dimensions.length) {
            throw new RuntimeException();
        }

        int[] result = new int[dimensions.length];

        for (int i = 0; i < dimensions.length; ++i) {
            result[i] = dimensions[i].indexOf(d[i]);
        }

        return result;
    }

    public boolean insert(double[] d, double v) {
        int index = oldIndexOf(d);

        if (index != -1) {
            insert(index, v);
            return true;
        }

        return false;
    }

    public void insert(int i, double v) {
        if (i < 0 || i >= values.length) {
            throw new RuntimeException();
        }

        ++writeCtr[i];
        values[i] = v;
    }    

    private int[] oldIndicesOf(double[] d) {
        if (d.length != dimensions.length) {
            throw new RuntimeException();
        }

        int[] result = new int[dimensions.length];

        for (int i = 0; i < dimensions.length; ++i) {
            int index = dimensions[i].oldIndexOf(d[i]);

            if (index == -1) {
                //System.out.println(dimensions[i]+" does not contain "+Utils.dTS(d[i]));
                return null;

            } else {
                result[i] = index;
            }
        }

        return result;
    }

    public int oldIndexOf(double[] d) {
        int[] i = oldIndicesOf(d);
        if (i != null) {
            return oldIndexOf(i);
        }

        return -1;
    }

    private int oldIndexOf(int[] indices) {
        int result = 0;

        for (int i = 0; i < indices.length; ++i) {
            result += (indices[i] * offsets[i]);
        }

        return result;
    }

    public double oldGetLinInt(double[] d) {
        return oldGetLinInt(d, oldIndicesOf(d), 0);
    }

    private double oldGetLinInt(double[] d, int[] indices, int dim) {
        int[] lower = Arrays.copyOf(indices, indices.length);
        int[] upper = Arrays.copyOf(indices, indices.length);

        double center = dimensions[dim].getCenter(indices[dim]);
        double[] bounds = dimensions[dim].getBoundaries(indices[dim]);
        double wl, wu;

        if (d[dim] < center) {
            lower[dim] = Math.max(0, lower[dim] - 1);
            wu = 0.5 + ((d[dim] - bounds[0]) / (center - bounds[0]) * 0.5);
            wl = 1 - wu;

        } else {
            upper[dim] = Math.min(dimensions[dim].getSize() - 1, upper[dim] + 1);
            wl = 1 - (((d[dim] - center) / (bounds[1] - center)) * 0.5);
            wu = 1 - wl;
        }

        if (dim + 1 == dimensions.length) {
            return values[oldIndexOf(lower)] * wl + values[oldIndexOf(upper)] * wu;

        } else {
            return oldGetLinInt(d, lower, dim + 1) * wl + oldGetLinInt(d, upper, dim + 1);
        }
    }

    public void reset() {
        Arrays.fill(values, Double.NaN);
        Arrays.fill(readCtr, 0);
        Arrays.fill(writeCtr, 0);
    }

    public void resetCounters() {
        Arrays.fill(readCtr, 0);
        Arrays.fill(writeCtr, 0);
    }

    public int getSize() {
        return values.length;
    }

    public double[][] getBoundaries(int index) {
        if (index < 0 || index >= values.length) {
            throw new RuntimeException();
        }

        double[][] result = new double[dimensions.length][2];

        for (int i = 0; i < this.dimensions.length; ++i) {
            int k = index / offsets[i];
            index -= k * offsets[i];

            double[] d = dimensions[i].getBoundaries(k);

            result[i][0] = d[0];
            result[i][1] = d[1];
        }

        return result;
    }

    public double usage() {
        int ctr = 0;

        for (int i = 0; i < values.length; ++i) {
            if (!Double.isNaN(values[i])) {
                ++ctr;
            }
        }

        return (double) ctr / (double) values.length;
    }

    public static LookUpTable load(InputStream in) {
        try {
            return LookUpTable.load(new BufferedReader(new InputStreamReader(in, "UTF-8")));
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static LookUpTable load(BufferedReader reader) {
        try {
            LookUpTable result = new LookUpTable();

            String line;

            // number of dimensions (first line) and dimensions (following lines)
            line = reader.readLine();

            result.dimensions = new Dimension[Integer.parseInt(line)];
            result.offsets = new int[result.dimensions.length];

            int size = 1;

            for (int i = 0; i < result.dimensions.length; ++i) {
                line = reader.readLine();
                result.dimensions[i] = new Dimension(line);
                size *= result.dimensions[i].getSize();
            }

            result.values = new double[size];
            result.readCtr = new int[size];
            result.writeCtr = new int[size];

            // offsets
            line = reader.readLine();
            StringTokenizer tokenizer = new StringTokenizer(line, " ");
            for (int i = 0; i < result.offsets.length; ++i) {
                result.offsets[i] = Integer.parseInt(tokenizer.nextToken());
            }

            // data
            line = reader.readLine();
            tokenizer = new StringTokenizer(line, " ");
            for (int i = 0; i < result.values.length; ++i) {
                result.values[i] = Double.parseDouble(tokenizer.nextToken());
            }

            line = reader.readLine();
            tokenizer = new StringTokenizer(line, " ");
            for (int i = 0; i < result.readCtr.length; ++i) {
                result.readCtr[i] = Integer.parseInt(tokenizer.nextToken());
            }

            line = reader.readLine();
            tokenizer = new StringTokenizer(line, " ");
            for (int i = 0; i < result.writeCtr.length; ++i) {
                result.writeCtr[i] = Integer.parseInt(tokenizer.nextToken());
            }

            reader.close();

            return result;

        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static LookUpTable load(String file) {
        try {
            return LookUpTable.load(new FileInputStream(file));
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public void write(java.io.OutputStreamWriter w)
            throws Exception {
        w.write(String.valueOf(dimensions.length));
        w.write("\n");
        for (int i = 0; i < dimensions.length; ++i) {
            dimensions[i].write(w);
            w.write("\n");
        }
        for (int i = 0; i < offsets.length; ++i) {
            w.write(String.valueOf(offsets[i]));
            w.write(" ");
        }
        w.write("\n");
        for (int i = 0; i < values.length; ++i) {
            w.write(String.valueOf(values[i]));
            w.write(" ");
        }
        w.write("\n");
        for (int i = 0; i < readCtr.length; ++i) {
            w.write(String.valueOf(readCtr[i]));
            w.write(" ");
        }
        w.write("\n");
        for (int i = 0; i < writeCtr.length; ++i) {
            w.write(String.valueOf(writeCtr[i]));
            w.write(" ");
        }
    }

    public void save(String file)
            throws Exception {
        FileOutputStream fo = new FileOutputStream(file);
        OutputStreamWriter w = new OutputStreamWriter(fo, "UTF-8");

        write(w);

        w.flush();
        fo.flush();
        w.close();
        fo.close();
    }

    public static String boundsToString(double[][] d) {
        StringBuffer result = new StringBuffer();

        for (int k = 0; k < d.length; ++k) {
            result.append("[");
            result.append(Utils.dTS(d[k][0]));
            result.append(", ");
            result.append(Utils.dTS(d[k][1]));
            result.append("[");
            if (k < d.length - 1) {
                result.append(" ");
            }
        }

        return result.toString();
    }

    public void print() {
        for(int i = 0; i < this.dimensions.length; ++i){
            System.out.println((i+1)+" "+dimensions[i]);
        }
        for (int i = 0; i < values.length; ++i) {
            double[][] d = getBoundaries(i);

            System.out.print(boundsToString(d));
            System.out.print(" " + Utils.dTS(values[i]));
            System.out.println("");
        }
    }

    public void paint(Graphics2D g, Color c, boolean paintBG) {
        java.awt.Dimension size = new java.awt.Dimension(400, 100);
        final int OFFSET = 30;
        final int SPACE = size.width - (OFFSET + OFFSET);
        int x;

        if (paintBG) {
            g.setColor(Color.WHITE);
            g.fillRect(0, 0, size.width, size.height);

            g.setColor(Color.BLACK);
            g.drawLine(OFFSET - 5, size.height / 2, size.width - OFFSET, size.height / 2);
            g.drawString("0", 15, (size.height / 2) + 6);
            g.drawLine(OFFSET, 10, OFFSET, size.height - 10);
            g.drawLine(OFFSET - 5, 10, OFFSET + 5, 10);
            g.drawString("1", 15, 15);
            g.drawLine(OFFSET - 5, size.height - 10, OFFSET + 5, size.height - 10);
            g.drawString("-1", 11, size.height - 7);

            x = OFFSET + (int) Math.round(100.0 / 350.0 * SPACE);
            g.drawLine(x, (size.height / 2) - 5, x, (size.height / 2) + 5);
            g.drawString("100km/h", x - 30, (size.height / 2) + 20);

            x = OFFSET + (int) Math.round(200.0 / 350.0 * SPACE);
            g.drawLine(x, (size.height / 2) - 5, x, (size.height / 2) + 5);
            g.drawString("200km/h", x - 30, (size.height / 2) + 20);

            x = OFFSET + (int) Math.round(300.0 / 350.0 * SPACE);
            g.drawLine(x, (size.height / 2) - 5, x, (size.height / 2) + 5);
            g.drawString("300km/h", x - 30, (size.height / 2) + 20);

            x = OFFSET + (int) Math.round(350.0 / 350.0 * SPACE);
            g.drawLine(x, (size.height / 2) - 5, x, (size.height / 2) + 5);
            g.drawString("350km/h", x - 20, (size.height / 2) + 20);
        }

        int x1, x2, y1, y2;
        g.setColor(c);
        x1 = OFFSET;
        x2 = OFFSET + (int) Math.round(34.0 / 350.0 * SPACE);
        y2 = (int) Math.round((size.height / 2) - (get(0) * ((size.height - 20) / 2)));
        g.drawLine(x1, y2, x2, y2);

        for (int i = 1; i < 5; ++i) {
            x1 = x2;
            y1 = y2;
            x2 = OFFSET + (int) Math.round((34.0 + (i * 68.0)) / 350.0 * SPACE);
            y2 = (int) Math.round((size.height / 2) - (get(i) * ((size.height - 20) / 2)));
            g.drawLine(x1, y1, x2, y2);
        }

        x1 = x2;
        y1 = y2;
        x2 = OFFSET + (int) Math.round(340.0 / 350.0 * SPACE);
        g.drawLine(x1, y1, x2, y2);
    }    

    public static void main(String[] args) {
//        try {
//            paintTables();
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
//        Dimension d1 = new Dimension("Speed", new double[]{0, 340}, 5);
//        Dimension d2 = new Dimension("Trackpos", new double[]{-1, 1}, 5);
//        Dimension d3 = new Dimension("Trackangle", new double[]{-9, 9}, 1);
//
//        LookUpTable lut = new LookUpTable(new Dimension[]{d1, d2, d3});

//        try {
//            lut.save("/home/quad/lut_test");
//
//        } catch (Exception e) {
//            e.printStackTrace();
//        }

    /*for(int i=0; i < lut.getSize(); ++i){
    double[][] d = lut.getBoundaries(i);

    for(int k=0; k < d.length; ++k){
    System.out.print("["+d[k][0]+", "+d[k][1]+"[ ");
    }
    System.out.println("");
    }*/

    /*lut = LookUpTable.load("/home/quad/experiments/test3/TYPE_FULL_acc");

    for (int i = 0; i < lut.getSize(); ++i) {
    double[][] d = lut.getBoundaries(i);

    for (int k = 0; k < d.length; ++k) {
    System.out.print("[" + d[k][0] + ", " + d[k][1] + "[ ");
    }
    System.out.println("");
    }*/

//        for (int i = 1; i < 330; ++i) {
//            System.out.println(Utils.dTS(1.0 * i) + ": " +
//                    Utils.dTS(lut.get(new double[]{1.0 * i})) + ", " +
//                    Utils.dTS(lut.oldGetLinInt(new double[]{1.0 * i})));
//
//        }

    //lut = LookUpTable.load("/home/quad/experiments/luthistexp/TYPE_STRAIGHT_steering_fast");
        /*lut = LookUpTable.load("/home/quad/ALL_FAST_steering");

    for (int i = 0; i < lut.getSize(); ++i) {
    System.out.print(i+" ");
    double[][] d = lut.getBoundaries(i);

    for (int k = 0; k < d.length; ++k) {
    System.out.print("[" + d[k][0] + ", " + d[k][1] + "[ ");
    }
    System.out.println(lut.get(i)+"");
    }*/

//        for (int i = 0; i < 20; ++i) {
//            double trackPos = -1 + (i*0.1);
//
//            for(int j=0; j < 18; ++j){
//                double trackAngle = -9 + (j*1.0);
//
//                double[] v = new double[2];
//                v[0] = trackPos;
//                v[1] = trackAngle;
//
//                System.out.println("("+Utils.dTS(trackPos)+", "+Utils.dTS(trackAngle)+"): " +
//                    Utils.dTS(lut.get(v)) + ", " +
//                    Utils.dTS(lut.oldGetLinInt(v)));
//            }
//        }

        java.io.InputStream in = (new Object()).getClass().getResourceAsStream("/de/janquadflieg/mrracer/data/cigtables/ALL_FAST_steering");
        LookUpTable lut = LookUpTable.load(in);

        // quadratisch
        for (int i = 0; i < 40; ++i) {
            double trackPos = -2 + (i * 0.1);

            if(i==0){
                System.out.println("             Trackangle");
                System.out.printf("%6s", "");
                for (int j = 0; j < 40; ++j) {
                    double trackAngle = -20 + (j * 1.0);
                    System.out.printf("%7s", Utils.dTS(trackAngle));
                }
                System.out.println("");
            }

            System.out.printf("%6s", Utils.dTS(trackPos)+" ");
            for (int j = 0; j < 40; ++j) {
                double trackAngle = -20 + (j * 1.0);

                double[] v = new double[2];
                v[0] = trackPos;
                v[1] = trackAngle;

                System.out.printf("%7s", Utils.dTS(lut.getLinInt(v))+" ");
            }
            System.out.println("");
        }
    }
}