/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.functions;

import java.io.*;
import java.awt.*;

/**
 *
 * @author Jan Quadflieg
 */
public class LinearFunction {

    /** Zero point of the function. */
    private double zeropoint = 0.0;
    /** Gradient. */
    private double gradient = -0.5;

    public LinearFunction(double zp, double g) {
        this.zeropoint = zp;
        this.gradient = g;
    }

    public double getValue(double x) {
        return gradient * (x - zeropoint);
    }

    public double getInverse(double y) {
        return (y / gradient) + zeropoint;
    }

    public void setZeroPoint(double d) {
        this.zeropoint = d;
    }

    public double getZeroPoint() {
        return zeropoint;
    }

    public double getGradient() {
        return gradient;
    }

    public void paint(Graphics2D g, Color c, boolean paintBG) {
        final Dimension size =new Dimension(400, 100);
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

        int x1, x2;
        g.setColor(c);
        x1 = OFFSET;
        x2 = OFFSET + (int) Math.round(getInverse(1) / 350.0 * SPACE);
        x2 = Math.min(size.width - OFFSET, Math.max(OFFSET, x2));
        if (x2 > x1) {
            g.drawLine(x1, 10, x2, 10);
        }
        x1 = x2;
        x2 = OFFSET + (int) Math.round(getInverse(-11) / 350.0 * SPACE);
        x2 = Math.min(size.width - OFFSET, Math.max(OFFSET, x2));
        if (x2 > x1) {
            g.drawLine(x1, 10, x2, size.height - 10);
        }
        x1 = x2;
        x2 = size.width - OFFSET;
        if (x2 > x1) {
            g.drawLine(x1, size.height - 10, x2, size.height - 10);
        }
    }

    public void setGradient(double d) {
        this.gradient = d;
    }

    public static LinearFunction load(InputStream in) {
        try {
            return LinearFunction.load(new BufferedReader(new InputStreamReader(in, "UTF-8")));
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static LinearFunction load(BufferedReader reader) {
        try {
            LinearFunction result = new LinearFunction(0, 1);

            String line;

            // zeropoint
            line = reader.readLine();
            result.zeropoint = Double.parseDouble(line);

            // gradient
            line = reader.readLine();
            result.gradient = Double.parseDouble(line);

            reader.close();

            return result;

        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static LinearFunction load(String file) {
        try {
            return LinearFunction.load(new FileInputStream(file));
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public void write(java.io.OutputStreamWriter w)
            throws Exception {
        w.write(String.valueOf(zeropoint));
        w.write("\n");
        w.write(String.valueOf(gradient));
        w.write("\n");
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

    public static void main(String[] args) {
//        LinearFunction f = new LinearFunction(200, (-2.0/100.0));
//        for(int i=0; i < 350; i += 10){
//            double y = f.getValue(i*1.0);
//            System.out.println(i+", "+y);
//            System.out.println(f.getInverse(y)+", "+y);
//            System.out.println(Math.min(1, Math.max(-1, y)));
//        }
        try {
            
        /*LinearFunction f = LinearFunction.load("D:\\Studium\\Diplomarbeit\\"+
        "Data\\Experiments\\lf-reflex-controller\\in\\TYPE_FULL_acc");
        System.out.println(f.getValue(34.0));
        System.out.println(f.getValue(102.0));
        System.out.println(f.getValue(170.0));
        System.out.println(f.getValue(238.0));
        System.out.println(f.getValue(306.0));
        System.out.println(f.getInverse(1.0));
        System.out.println(f.getInverse(0.0));
        System.out.println(f.getInverse(-1.0));
        BufferedImage img = new BufferedImage(400, 100, BufferedImage.TYPE_INT_RGB);
        f.paint(img.createGraphics(), new Dimension(400, 100), Color.MAGENTA);
        ImageIO.write(img, "PNG", new File("c:\\test.png"));*/



        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
