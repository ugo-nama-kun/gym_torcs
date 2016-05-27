package scr;

import java.util.Hashtable;
import java.util.Enumeration;
import java.util.StringTokenizer;

/**
 * Created by IntelliJ IDEA.
 * User: Administrator
 * Date: Feb 22, 2008
 * Time: 6:17:32 PM
 */
public class MessageParser {
    // parses the message from the serverbot, and creates a table of
    // associated names and values of the readings

    private Hashtable<String, Object> table = new Hashtable<String, Object>();
    private String message;
    
    public MessageParser(String message) {
    	this.message = message;
        //System.out.println(message);
        StringTokenizer mt = new StringTokenizer(message, "(");
        while (mt.hasMoreElements()) {
            // process each reading
            String reading = mt.nextToken();
            //System.out.println(reading);
            int endOfMessage = reading.indexOf(")");
            if (endOfMessage > 0) {
                reading = reading.substring(0, endOfMessage);
            }
            StringTokenizer rt = new StringTokenizer(reading, " ");
            if (rt.countTokens() < 2) {
                //System.out.println("Reading not recognized: " + reading);
            } else {
                String readingName = rt.nextToken();
                Object readingValue = "";
                if (readingName.equals("opponents") || readingName.equals("track") ||
                        readingName.equals("wheelSpinVel")|| readingName.equals("focus") ) {//ML
                    // these readings have multiple values
                    readingValue = new double[rt.countTokens()];
                    int position = 0;
                    //System.out.println(readingName);
                    while (rt.hasMoreElements()) {
                        String nextToken = rt.nextToken ();
                        //System.out.print (nextToken);
                        try{
                            ((double[]) readingValue)[position] = Double.parseDouble(nextToken);
                         }
                        catch (Exception e)
                        {
                            System.out.println("Error parsing value '"+nextToken+"' for "+ readingName+" using 0.0");
                            System.out.println("Message: "+message);
                            ((double[]) readingValue)[position] = 0.0;
                        }
                        //System.out.println(" "   +position +" " + ((double[])readingValue)[position]);
                        position++;
                    }
                } else {
                    String token = rt.nextToken ();
                    try{
                        readingValue = new Double (token);
                    }
                    catch (Exception e)
                    {
                        System.out.println("Error parsing value '"+token+"' for "+ readingName+" using 0.0");
                        System.out.println("Message: "+message);
                        readingValue = new Double(0.0);
                    }
                }
                table.put(readingName, readingValue);
            }
        }
    }

    public void printAll() {
        Enumeration<String> keys = table.keys();
        while (keys.hasMoreElements()) {
            String key = keys.nextElement();
            System.out.print(key + ":  ");
            System.out.println(table.get(key));
        }
    }

    public Object getReading(String key) {
        return table.get(key);
    }

	public String getMessage() {
		return message;
	}
}
