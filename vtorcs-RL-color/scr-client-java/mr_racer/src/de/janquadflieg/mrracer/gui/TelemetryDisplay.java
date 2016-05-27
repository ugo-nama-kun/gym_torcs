/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.gui;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.telemetry.*;

import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.image.*;
import javax.swing.*;

/**
 *
 * @author Jan Quadflieg
 */
public class TelemetryDisplay
extends JComponent
implements TelemetryListener{

    /** The telemetry object belonging to this display. */
    private Telemetry telemetry;

    /** First offscreen buffer. */
    private BufferedImage buffer1 = new BufferedImage(1,1,BufferedImage.TYPE_INT_ARGB);

    /** Second offscreen buffer. */
    private BufferedImage buffer2 = new BufferedImage(1,1,BufferedImage.TYPE_INT_ARGB);;

    /** The front buffer. */
    private BufferedImage front = buffer1;

    /** The back buffer. */
    private BufferedImage back = buffer2;

    /** SensorData. */
    private ModifiableSensorData oldSData = new ModifiableSensorData();

    /** SensorData.*/
    private ModifiableSensorData toDrawSOld = new ModifiableSensorData();

    /** SensorData.*/
    private ModifiableSensorData toDrawSNew = new ModifiableSensorData();

    /** ActionData.*/
    private ModifiableAction oldAData = new ModifiableAction();

    /** ActionData.*/
    private ModifiableAction toDrawAOld = new ModifiableAction();

    /** ActionData.*/
    private ModifiableAction toDrawANew = new ModifiableAction();

    /** Updater. */
    private Updater updater;

    /** The size and position of the sensor data area. */
    private Rectangle sensorarea = new Rectangle();

    /** The size and position of the action data area. */
    private Rectangle actionarea = new Rectangle();

    /** The Current x coordinate. */
    private int x = 0;

    /** Resolution (meter per pixel). */
    private double mpp = 1.0;

    /** X Increment. */
    private final static int X_INC = 2;

    /** The current state of the display. */
    private int state;

    /** State indicating that the buffers are recreated. */
    private static final int RESET_BUFFERS = 0x000001;

    /** State indicating that a complete redraw takes place. */
    private static final int COMPLETE_REPAINT = 0x000002;

    /** State indicatind that data just gets appended. */
    private static final int PAINT_APPEND = 0x000004;
    
    public TelemetryDisplay(Telemetry t){
        super();

        this.telemetry = t;

        telemetry.addListener(this);

        this.addComponentListener(new ComponentAdapter(){
            @Override
            public void componentResized(ComponentEvent e){                
                componentResizedImpl();
            }
        });

        updater = new Updater(this);
    }

    private synchronized void componentResizedImpl(){
        state = RESET_BUFFERS;
        updater.interrupt();
    }

    @Override
    public void newLap(){
        synchronized(this){
            state = COMPLETE_REPAINT;
        }
    }

    @Override
    public void cleared(){
        synchronized(this){
            oldSData.setData(new SensorData());
            toDrawSOld.setData(new SensorData());
            toDrawSNew.setData(new SensorData());
            state = COMPLETE_REPAINT;
            updater.interrupt();
        }
    }

    @Override
    public void newData(SensorData model, de.janquadflieg.mrracer.telemetry.Action a, String l, Telemetry.Mode mode){
        if(oldSData.getTimeStamp() == Utils.NO_DATA_L){
            oldSData.setData(model);
            oldAData.setData(a);

        //} else if(model.getTimeStamp()-oldSData.getTimeStamp() > 250){
        } else if(model.getDistanceRaced()-oldSData.getDistanceRaced() > (2*mpp)){
            synchronized(this){
                toDrawSOld.setData(oldSData);
                toDrawSNew.setData(model);

                toDrawAOld.setData(oldAData);
                toDrawANew.setData(a);
            }
            oldSData.setData(model);
            oldAData.setData(a);
            updater.interrupt();
        }
    }


    @Override
    public void modeChanged(int newMode){
        
    }

    private synchronized void copyToBackBuffer(){
        Graphics2D g = this.back.createGraphics();
        g.setColor(Color.WHITE);
        g.fillRect(0, 0, front.getWidth(), front.getHeight());
        g.drawImage(front, 0, 0, this);
    }

    private synchronized void asyncRepaint(){
        Graphics2D g = this.back.createGraphics();

        g.setColor(Color.WHITE);
        g.fillRect(0, 0, getWidth(), getHeight());

        g.setColor(Color.BLACK);
        g.drawString("Current lap: "+telemetry.getCurrentLap(), 200, 60);

        g.setColor(Color.LIGHT_GRAY);
        g.fill(sensorarea);
        g.fill(actionarea);

        this.mpp = telemetry.getTrackLength() / this.getWidth();

        x = 0;

        swapBuffers();

        copyToBackBuffer();       
    }

    @Override
    public void paintComponent(Graphics graphics) {
        Graphics2D g = (Graphics2D) graphics;

        g.setColor(Color.WHITE);

        g.fillRect(0, 0, getWidth(), getHeight());

        synchronized(this){
            g.drawImage(front, 0, 0, this);
        }
    }
    
    private synchronized void incPaint(){
        Graphics2D g = this.back.createGraphics();
        
        // sensors
        g.translate(sensorarea.x, sensorarea.y);
        incPaintSpeed(g, sensorarea.height);
        incPaintLatSpeed(g, sensorarea.height);
        g.translate(-sensorarea.x, -sensorarea.y);

        // actions
        g.translate(actionarea.x, actionarea.y);
        incPaintBrake(g, actionarea.height);
        incPaintAccelerate(g, actionarea.height);
        incPaintSteering(g, actionarea.height);
        g.translate(-actionarea.x, -actionarea.y);

        x+=2;

        swapBuffers();

        copyToBackBuffer();
    }

    private void incPaintSpeed(Graphics2D g, int height){
        g.setColor(Color.GREEN);
        double f1 = toDrawSOld.getSpeed() / (double)height;
        int y1 = (int) Math.round(f1*height);
        double f2 = toDrawSNew.getSpeed() / (double)height;
        int y2 = (int) Math.round(f2*height);

        g.drawLine(x, height-y1, x+2, height-y2);        
    }

    private void incPaintLatSpeed(Graphics2D g, int height){
        g.setColor(Color.ORANGE);
        double f1 = toDrawSOld.getLateralSpeed() / (double)height;
        int y1 = (int) Math.round(f1*height);
        double f2 = toDrawSNew.getLateralSpeed() / (double)height;
        int y2 = (int) Math.round(f2*height);

        g.drawLine(x, height-y1, x+2, height-y2);
    }

    private void incPaintBrake(Graphics2D g, int height){
        g.setColor(Color.RED);
        int y1 = (int) Math.round(toDrawAOld.getBrake() * (height / 2));
        int y2 = (int) Math.round(toDrawANew.getBrake() * (height / 2));

        g.drawLine(x, (height/2)+y1, x+2, (height/2)+y2);
    }

    private void incPaintAccelerate(Graphics2D g, int height){
        g.setColor(Color.GREEN);
        int y1 = (int) Math.round(toDrawAOld.getAcceleration() * (height / 2));
        int y2 = (int) Math.round(toDrawANew.getAcceleration() * (height / 2));

        g.drawLine(x, (height/2)-y1, x+2, (height/2)-y2);
    }

    private void incPaintSteering(Graphics2D g, int height){
        g.setColor(Color.BLUE);
        int y1 = (int) Math.round(toDrawAOld.getSteering() * -1 * (height / 2));
        int y2 = (int) Math.round(toDrawANew.getSteering() * -1 * (height / 2));

        g.drawLine(x, (height/2)+y1, x+2, (height/2)+y2);
    }

    private synchronized void resetBuffers(){
        this.buffer1 = new BufferedImage(getWidth(), getHeight(), BufferedImage.TYPE_INT_ARGB);
        this.buffer2 = new BufferedImage(getWidth(), getHeight(), BufferedImage.TYPE_INT_ARGB);
        this.front = buffer1;
        this.back = buffer2;

        sensorarea.setBounds(0, 100, getWidth(), 400);
        actionarea.setBounds(0, 650, getWidth(), 200);

        Graphics2D g = this.front.createGraphics();
        g.setColor(Color.WHITE);
        g.fillRect(0, 0, getWidth(), getHeight());
        g.setColor(Color.BLACK);
        g.drawString("resetBuffers: buffer 1", 30, 30);
        g.drawString(getWidth()+" / "+getHeight(), 30, 60);
        

        g = this.back.createGraphics();
        g.setColor(Color.WHITE);
        g.fillRect(0, 0, getWidth(), getHeight());
        g.setColor(Color.BLACK);
        g.drawString("resetBuffers: buffer 2", 30, 30);
        g.drawString(getWidth()+" / "+getHeight(), 30, 60);
        //g.drawImage(front, 0, 0, this);
        //g.drawString("This is the backbuffer", 30, 90);
        //g.setColor(Color.LIGHT_GRAY);
        //g.fill(sensorarea);
        //g.fill(actionarea);
    }

    private synchronized void swapBuffers(){
        if(front == buffer1){
            front = buffer2;
            back = buffer1;

        } else {
            front = buffer1;
            back = buffer2;
        }
    }

//    public void setSensorData(SensorData model) {
//
//    }

    private static class Updater
    implements Runnable{
        private TelemetryDisplay peer = null;
        private Thread t = null;
        private boolean keepRunning = true;

        public Updater(TelemetryDisplay d){
            this.peer = d;
            t = new Thread(this, "Telemetry Updater");
            t.start();
        }

        public void interrupt(){
            t.interrupt();
        }

        @Override
        public void run(){
            while(true){
                try{
                    if(peer.state == TelemetryDisplay.COMPLETE_REPAINT){
                        System.out.println("Uh, kompletter repaint nötig");
                        peer.asyncRepaint();
                        System.out.println("Fertig, wechsle wieder auf append");
                        peer.state = TelemetryDisplay.PAINT_APPEND;

                    } else if(peer.state == TelemetryDisplay.RESET_BUFFERS){
                        peer.resetBuffers();
                        peer.asyncRepaint();
                        peer.state = TelemetryDisplay.PAINT_APPEND;

                    } else if(peer.state == TelemetryDisplay.PAINT_APPEND){
                        peer.incPaint();
                    }
                    
                    peer.repaint();
                    Thread.sleep(20000);

                } catch(InterruptedException e){
                    //e.printStackTrace();
                }
            }
        }

        public void stop(){
            keepRunning = false;
            t.interrupt();
        }
    }
}
