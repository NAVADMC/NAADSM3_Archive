/**
 * A very simple class that validates an XML document.
 *
 * @author Neil Harvey
 *   Grid Computing Research Group
 *   Department of Computing & Information Science, University of Guelph
 *   Guelph, ON N1G 2W1
 *   CANADA
 *   nharvey@uoguelph.ca
 * @version 0.1
 * @date February 2003
 *
 * Copyright (c) University of Guelph, 2003
 */
import javax.xml.parsers.*;
import org.w3c.dom.*;
import java.io.*;

public class XMLValidator
{
  public static void main (String args[])
  {
    try
    {
      DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
      dbf.setValidating (true);
      DocumentBuilder db = dbf.newDocumentBuilder();
      // Check to make sure the parser supports validation.
      System.out.println ("Validating parser? " + db.isValidating());
      Document d = db.parse (new File (args[0]));
    }
    catch (Exception e)
    {
      System.out.println (e);
    }
  }
}
