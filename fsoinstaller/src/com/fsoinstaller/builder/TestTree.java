
package com.fsoinstaller.builder;

import javax.swing.JFrame;
import javax.swing.JTree;

import com.fsoinstaller.common.InstallerNode;
import com.fsoinstaller.common.InstallerNodeRoot;
import com.fsoinstaller.common.InstallerNodeTreeModel;


public class TestTree extends JFrame
{
	public TestTree()
	{
		InstallerNodeRoot root = new InstallerNodeRoot();
		root.addChild(new InstallerNode("fsport"));
		root.addChild(new InstallerNode("inferno"));
		root.addChild(new InstallerNode("scroll"));
		root.getChildren().get(0).addChild(new InstallerNode("fsport-mediavps"));
		root.getChildren().get(0).addChild(new InstallerNode("fsport-str"));

		JTree tree = new JTree(new InstallerNodeTreeModel(root));
		add(tree);
	}

	public static void main(String[] args)
	{
		JFrame frame = new TestTree();
		frame.pack();
		frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		frame.setVisible(true);
	}
}
