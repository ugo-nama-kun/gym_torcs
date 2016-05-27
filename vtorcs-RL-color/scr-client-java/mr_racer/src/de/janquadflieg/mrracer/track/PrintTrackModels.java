/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.track;

import java.awt.*;
import java.awt.geom.*;
import java.awt.image.*;
import java.io.*;
import javax.imageio.ImageIO;

/**
 *
 * @author quad
 */
public class PrintTrackModels {

    //private static final Color DARK_GREEN = new Color(0, 129, 36);
    public static void paintModel(TrackModel m, BufferedImage img) {
        //m.print();
        Graphics2D g = img.createGraphics();
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        final int w = img.getWidth();
        final int h = img.getHeight();
        final int dh = 10;
        final boolean outline = true; // rectangle around segments?

        Font font = new Font(null, Font.BOLD, 30);
        g.setFont(font);

        g.setColor(Color.WHITE);
        g.fillRect(0, 0, w, h);

        g.setColor(Color.BLACK);
        //String header = "Trackmodel of Wheel2 wo noise";
        //String header = "Trackmodel of Wheel2 with noise & noise handling by quadratic regression";
        String header = "Trackmodel of Wheel2 with noise & naive noise handling by averaging";
        
        Rectangle2D strBounds = g.getFontMetrics().getStringBounds(header, g);
        g.drawString(header, Math.round((w - strBounds.getWidth()) / 2.0), (int) Math.ceil(strBounds.getHeight()));

        double ppm = (w - 5) / m.getLength();
        int rh = h - ((int) Math.ceil(strBounds.getHeight()) + 5);
        int sh = rh - 100; // segment height
        g.translate(0, (int) Math.ceil(strBounds.getHeight()) + 5);

        

        int offset = 0;
        int yoff = 0;
        for (int i = 0; i < m.size(); ++i) {
            TrackSegment s = m.getSegment(i);

            if (s.isForward()) {
                g.setColor(Color.green);

            } else if (s.isLeft()) {
                g.setColor(Color.DARK_GRAY);

            } else if (s.isRight()) {
                g.setColor(Color.BLUE);

            } else if (s.isUnknown()) {
                g.setColor(Color.red);
            }

            int width = (int) Math.round(s.getLength() * ppm);
            //g.fillRect(offset, sh-dh, width, dh);

            int offset2 = offset;
            double position = s.getStart();
            for (int k = 0; k < s.size(); ++k) {
                TrackSubSegment sub = s.getSubSegment(k);
                int width2 = (int) Math.round(sub.getLength() * ppm);
                if (width2 == 0) {
                    width2 = 1;
                    position += 1 / ppm;
                    k = s.getIndex(position);
                }

                if (sub.isFull()) {
                    g.setColor(Color.YELLOW);

                } else if (sub.isMedium()) {
                    g.setColor(Color.ORANGE);

                } else if (sub.isSlow()) {
                    g.setColor(Color.RED);

                } else if (sub.isHairpin()) {
                    g.setColor(Color.PINK);
                }

                g.fillRect(offset2, 0, width2, sh - dh);

                offset2 += width2;
            }

            if (outline) {
                g.setColor(Color.BLACK);
                g.drawRect(offset, 0, width - 1, sh - dh);
                g.drawRect(offset + 1, 1, width - 2, sh - dh - 2);
            }

            String name = "";
            /*if (s.isLeft()) {
                name += "L";

            } else if (s.isRight()) {
                name += "R";

            } else if (s.isUnknown()) {
                name += "U";
            }*/
            name += String.valueOf(i + 1);
            g.setColor(Color.BLACK);
            strBounds = g.getFontMetrics().getStringBounds(name, g);
            //System.out.println(strBounds);

            if(yoff != 0){
                yoff = 0;

            } else {
                if(strBounds.getWidth() > width){
                    yoff = -10 + (int) Math.ceil(strBounds.getHeight()) ; //(int)Math.ceil(strBounds.getHeight());
                }
            }
            
            /*if(i%2 == 0){
                yoff = (int) Math.ceil(strBounds.getHeight()) ; //(int)Math.ceil(strBounds.getHeight());
            }*/
            //g.drawString(name, offset+Math.round((width - strBounds.getWidth()) / 2.0), (sh / 2) + (int) Math.ceil(strBounds.getHeight()*0.5)+yoff);
            g.drawString(name, offset+Math.round((width - strBounds.getWidth()) / 2.0), -10 + sh - dh + (int) Math.ceil(strBounds.getHeight())+yoff);

            offset += width;
        }

        /*double[] rawData = m.getRawData();
        g.translate(0, sh);        
        g.setColor(Color.black);
        if(rawData != null){
        for(int i=0; i < rawData.length; ++i){
        double absv = Math.abs(rawData[i]);
        int y = 99-(int)Math.round(absv);
        int x = (int)Math.round(1.0*i*ppm);
        g.fillRect(x, y, 1, 1);
        }
        }*/
    }

    public static void graphics() {
        //java.io.File currentDirectory = new java.io.File(".");
        //java.io.File currentDirectory = new java.io.File("F:\\Quad\\svn\\Diplomarbeit\\Data\\Tracks\\NewTrackModels.Unused");
        java.io.File currentDirectory = new java.io.File("F:\\Quad\\svn\\Torcs-Paper\\TORCS\\CIG-2011\\experiments\\strecken");
        java.io.File[] files = currentDirectory.listFiles();

        final int WIDTH = 1500;
        final int HEIGHT = 250;

        //String theFile = "Wheel2_noisy_final.saved_model";
        //String theFile = "wheel2-nonoise-cig-paper.saved_model";
        //String theFile = "wheel2-final-noise-handling.saved_model";
        String theFile = "wheel2_noisy_naive.saved_model";

        for (java.io.File f : files) {
            if (f.isFile() && f.getName().endsWith(TrackModel.TM_EXT)
                    && (theFile == null || theFile.equals(f.getName()))) {
                try {
                    String filename = currentDirectory.getAbsolutePath() + java.io.File.separator + f.getName();
                    System.out.println("Loading trackmodel from file \""
                            + filename + "\".");
                    TrackModel tm = TrackModel.load(filename);

                    BufferedImage img = new BufferedImage(WIDTH, HEIGHT, BufferedImage.TYPE_INT_RGB);
                    paintModel(tm, img);

                    ImageIO.write(img, "png", new File(currentDirectory.getAbsolutePath() + File.separator
                            + f.getName() + ".png"));

                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public static void text() {
        java.io.File currentDirectory = new java.io.File(".");
        java.io.File[] files = currentDirectory.listFiles();

        for (java.io.File f : files) {
            if (f.isFile() && f.getName().endsWith(TrackModel.TM_EXT)) {
                try {
                    String filename = "." + java.io.File.separator + f.getName();
                    System.out.println("Loading trackmodel from file \""
                            + filename + "\".");
                    TrackModel tm = TrackModel.load(filename);
                    tm.print();

                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public static void main(String[] args) {
        //graphics();
        text();
    }
}
