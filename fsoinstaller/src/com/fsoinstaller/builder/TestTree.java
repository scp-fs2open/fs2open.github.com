
package com.fsoinstaller.builder;

import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Reader;
import java.io.Writer;

import javax.swing.JFrame;
import javax.swing.JTree;

import com.fsoinstaller.common.InstallerNode;
import com.fsoinstaller.common.InstallerNodeFactory;
import com.fsoinstaller.common.InstallerNodeParseException;
import com.fsoinstaller.common.InstallerNodeRoot;
import com.fsoinstaller.common.InstallerNodeTreeModel;


public class TestTree extends JFrame
{
	public TestTree() throws IOException, InstallerNodeParseException
	{
		Reader reader = new FileReader("fsport.txt");
		Writer writer = new FileWriter("fsport-new.txt");
		
		InstallerNodeRoot root = new InstallerNodeRoot();
		InstallerNode fsport = InstallerNodeFactory.readNode(reader);
		root.addChild(fsport);

		JTree tree = new JTree(new InstallerNodeTreeModel(root));
		add(tree);
		
		reader.close();
		
		InstallerNodeFactory.writeNode(writer, fsport);
		
		writer.close();
	}

	public static void main(String[] args) throws IOException, InstallerNodeParseException
	{
		JFrame frame = new TestTree();
		frame.pack();
		frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		frame.setVisible(true);
	}
}
