/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer;

import java.io.*;
import java.net.*;
import java.util.*;
import java.util.jar.*;

/**
 *
 * @author quad
 */
public class Configuration {

    /** Mac address desktop. */
    private static final byte[] DESKTOP_MAC_ADDRESS = {0, 24, -13, 112, 25, -124};

    /** Mac address thinkpad. */
    private static final byte[] THINKPAD_MAC_ADDRESS = {0, 9, 107, -6, 33, 108};

    /** Uni ip. */
    private static final byte[] UNI_NETWORK = {-127, -39, 38};

    public static final int UNKNOWN_COMPUTER = 0;

    public static final int DESKTOP = 1;

    public static final int THINKPAD = 2;

    public static final int HIWI_POOL = 3;

    public static final String DATA_PATH_DESKTOP = "G:"+File.separator+
            "Studium"+File.separator+"Diplomarbeit"+File.separator+"Data";

    public static final String DATA_PATH_THINKPAD = "D:"+File.separator+
            "Studium"+File.separator+"Diplomarbeit"+File.separator+"Data";

    public static final String DATA_PATH_HIWI_POOL = File.separator+
            "home"+File.separator+"quad"+File.separator+"Diplomarbeit"+
            File.separator+"Data";

    private String ipPort = "127.0.0.1:3001";

    private int computer = UNKNOWN_COMPUTER;

    private boolean runsFromJar = false;

    private Vector<ControllerInfo> controllers = new Vector<ControllerInfo>();

    private String execPath = "";

    private String jarName = "";

    public void init(){
        identifyRuntimeEnv();
        identifyComputer();
        enumerateControllers();
    }

    private void identifyRuntimeEnv(){
        execPath = System.getProperty("user.dir");
        URL url = ClassLoader.getSystemResource("de/janquadflieg/mrracer/Configuration.class");

        if(url != null){
            String path = url.toString();

            //System.out.println(path);

            runsFromJar = path.startsWith("jar:");

            if(runsFromJar){
                String s = path.substring(0, path.lastIndexOf(".jar!/"));
                jarName = (s.substring(s.lastIndexOf('/') + 1)) + ".jar";
                //System.out.println(jarName);
            }
            
        } else {
            System.out.println("Unable to detect the runtime environment.");
            System.exit(1);
        }
    }

    private void identifyComputer(){
        try {
            Enumeration<NetworkInterface> list = NetworkInterface.getNetworkInterfaces();

            while (list.hasMoreElements()) {
                boolean equal = false;
                NetworkInterface i = list.nextElement();
                byte[] macAddr = i.getHardwareAddress();

//                System.out.print("Adresse: ");
//                for(int j=0; macAddr != null && j < macAddr.length; ++j){
//                    System.out.print(macAddr[j]+";");
//                }
//                System.out.println("");

                equal = (macAddr != null);
                for (int j = 0; macAddr != null && j < macAddr.length && j < DESKTOP_MAC_ADDRESS.length; ++j) {
                    equal &= (macAddr[j] == DESKTOP_MAC_ADDRESS[j]);
                }

                if (equal) {
                    //System.out.println("Found desktop");
                    computer = DESKTOP;
                    break;
                }

                equal = (macAddr != null);
                for (int j = 0; macAddr != null && j < macAddr.length && j < THINKPAD_MAC_ADDRESS.length; ++j) {
                    equal &= (macAddr[j] == THINKPAD_MAC_ADDRESS[j]);
                }

                if (equal) {
                    //System.out.println("Found thinkpad");
                    computer = THINKPAD;
                    break;
                }

                Enumeration<InetAddress> ipAddrs = i.getInetAddresses();
                while(ipAddrs.hasMoreElements()){
//                    System.out.print("Adresse: ");
                    byte[] b = ipAddrs.nextElement().getAddress();
//                    for(int j=0; j < b.length; ++j){
//                        System.out.print(b[j]+";");
//                    }
//                    System.out.println("");
                    equal = true;
                    for(int j=0; j < b.length && j < UNI_NETWORK.length; ++j){
                        equal &= (b[j] == UNI_NETWORK[j]);
                    }
                    if(equal){
                        computer = HIWI_POOL;
                        //ipPort = "129.217.38.228:3001";
                        ipPort = "127.0.0.1:3010";
                        break;
                    }
                }

                if(equal){
                    break;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void enumerateControllers(){
        Vector<String> buffer = new Vector<String>();

        Class controllerClass10 = null;

        try{            
            controllerClass10 = Class.forName("champ2011client.Controller");

        } catch(Exception e){
            // this is a critical Exception!
            e.printStackTrace();
            System.exit(1);
        }

        if(runsFromJar){
            try{
                JarFile jarFile = new JarFile(execPath+
                File.separatorChar+jarName);

                Enumeration<JarEntry> en = jarFile.entries();

                while(en.hasMoreElements()){
                    JarEntry entry = en.nextElement();

                    String name = entry.getName();

                    if(!entry.isDirectory() && name.startsWith("de/janquadflieg/mrracer/controller") &&
                    name.endsWith(".class")){
                        String className = name.substring(name.lastIndexOf('/')+1, name.length() - 6);
                        buffer.add(className);
                    }
                }
            } catch(IOException e){
                e.printStackTrace();
            }

        } else {            
            String controllerDir = execPath;

            //System.out.println(execPath);

            if(execPath.endsWith("testenv")){
                controllerDir = "/home/quad/Diplomarbeit/Code/projects/MrRacer";
            }

            // just in case that the project is run from Netbeans
            if(controllerDir.endsWith("MrRacer")){
                controllerDir += File.separator+"build"+File.separator+"classes";
            }

            controllerDir += File.separator+"de"+File.separator+"janquadflieg"+
                    File.separator+"mrracer"+File.separator+"controller";

            File directory = new File(controllerDir);
            File[] files = directory.listFiles();

            for(int i=0; i<files.length; i++){
                String name = files[i].getName();

                if(!files[i].isDirectory() && name.endsWith(".class")){
                    String className = name.substring(0, name.length()-6);
                    buffer.add(className);
                }
            }
        }

        Collections.sort(buffer);

        for(String s: buffer){
            //System.out.println(s);
            try{
                Class c = Class.forName("de.janquadflieg.mrracer.controller."+s);                

                if(controllerClass10.isAssignableFrom(c) && !java.lang.reflect.Modifier.isAbstract(c.getModifiers())){
                    controllers.add(new ControllerInfo(s, true));
                }
            
            } catch(Exception e){
                e.printStackTrace();
            }
        }        
    }

    public String getDataPath(){
        if(computer == HIWI_POOL){
            return DATA_PATH_HIWI_POOL;

        } else if(computer == DESKTOP){
            return DATA_PATH_DESKTOP;

        } else if(computer == THINKPAD){
            return DATA_PATH_THINKPAD;

        } else {
            return "";
        }
    }

    public int getComputer(){
        return computer;
    }

    public String getServerIP(){
        return ipPort;
    }

    public Iterator<ControllerInfo> getControllers(){
        return controllers.iterator();
    }

    public static class ControllerInfo{
        public String name = "";
        public boolean i2010 = false;

        public ControllerInfo(String s, boolean b){
            this.name = s;
            this.i2010 = b;
        }

        public String toString(){
            return name;
        }
    }
}
