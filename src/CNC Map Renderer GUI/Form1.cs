using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;
using System.Diagnostics;

namespace RA2Maps_GUI {
	public partial class MainForm : Form {
		public MainForm() {
			InitializeComponent();
		}

		private void MainForm_Load(object sender, EventArgs e) {
			textBox3.Text = getrendererexe();
			textBox2.Text = readmixdir();
			UpdateCmd();
		}

		private string getrendererexe() {
			try {
				RegistryKey k = Microsoft.Win32.Registry.LocalMachine.OpenSubKey("SOFTWARE\\CNC Map Render");
				string s = (string)k.GetValue("");
				k.Close();
				return s + "\\cncmaprender.exe";

			}
			catch {
				return System.IO.File.Exists("cncmaprender.exe") ? "cncmaprender.exe" : "";
			}
		}

		private void radioButton1_CheckedChanged(object sender, EventArgs e) {
			textBox4.Visible = radioButton2.Checked;
			UpdateCmd();
		}

		private void radioButton2_CheckedChanged(object sender, EventArgs e) {
			textBox4.Visible = radioButton2.Checked;
			UpdateCmd();
		}

		private void UpdateCmd() {
			string cmd = getcmd();
			string file = textBox3.Text;
			if (file.Contains("\\"))
				file = file.Substring(file.LastIndexOf('\\') + 1);
			textBox5.Text = file + " " + cmd;
		}

		private string getcmd() {
			string cmd = string.Empty;

			cmd += "-i \"" + textBox1.Text + "\" ";
			if (PNG.Checked) {
				cmd += "-p ";
				if (numericUpDown1.Value != 6)
					cmd += "-c " + numericUpDown1.Value.ToString() + " ";
			}

			if (radioButton2.Checked) cmd += "-o \"" + textBox4.Text + "\" ";
			if (checkBox1.Checked) {
				cmd += "-j ";
				if (numericUpDown2.Value != 90)
					cmd += "-q " + numericUpDown2.Value.ToString() + " ";
			}

			if (textBox2.Text != readmixdir()) cmd += "-m " + "\"" + textBox2.Text + "\" ";
			if (checkBox3.Checked) cmd += "-r ";
			if (checkBox2.Checked) cmd += "-s ";
			if (checkBox4.Checked) cmd += "-S ";
			if (radioButton3.Checked) cmd += "-Y ";
			else if (radioButton5.Checked) cmd += "-y ";
			if (radioButton8.Checked) cmd += "-f ";
			if (radioButton8.Checked) cmd += "-F ";


			return cmd;
		}

		private string readmixdir() {
			try {
				RegistryKey k = Microsoft.Win32.Registry.LocalMachine.OpenSubKey("SOFTWARE\\Westwood\\Red Alert 2");
				string s = (string)k.GetValue("InstallPath");
				k.Close();
				return s.Substring(0, s.LastIndexOf('\\'));
			}
			catch {
				return "";
			}
		}

		private void textBox1_TextChanged(object sender, EventArgs e) {
			UpdateCmd();
		}

		private void checkBox3_CheckedChanged(object sender, EventArgs e) {
			UpdateCmd();
		}

		private void PNG_CheckedChanged(object sender, EventArgs e) {
			numericUpDown1.Visible = label1.Visible = PNG.Checked;
			UpdateCmd();
		}

		private void checkBox1_CheckedChanged(object sender, EventArgs e) {
			label2.Visible = numericUpDown2.Visible = checkBox1.Checked;
			UpdateCmd();
		}

		private void textBox4_TextChanged(object sender, EventArgs e) {
			UpdateCmd();
		}

		private void button1_Click(object sender, EventArgs e) {
			openFileDialog1.CheckFileExists = true;
			openFileDialog1.Multiselect = false;
			openFileDialog1.Filter = "RA2/YR map files (*.map, *.mpr, *.mmx, *.yrm, *.yro)|*.mpr;*.map;*.mmx;*.yrm;*.yro|All files (*.*)|*";
			openFileDialog1.InitialDirectory = readmixdir();
			openFileDialog1.FileName = "";
			if (openFileDialog1.ShowDialog() == DialogResult.OK)
				textBox1.Text = openFileDialog1.FileName;
		}

		private void checkBox4_CheckedChanged_1(object sender, EventArgs e) {
			if (checkBox4.Checked)
				checkBox2.Checked = false;
			UpdateCmd();
		}

		private void checkBox2_CheckedChanged(object sender, EventArgs e) {
			if (checkBox2.Checked)
				checkBox4.Checked = false;
			UpdateCmd();
		}

		private void numericUpDown1_ValueChanged(object sender, EventArgs e) {
			UpdateCmd();
		}

		private void button4_Click(object sender, EventArgs e) {
			if (System.IO.File.Exists(textBox1.Text) == false) {
				MessageBox.Show("Input file doesn't exist. Aborting.");
				return;
			}

			if (System.IO.File.Exists(textBox2.Text + "\\ra2.mix") == false) {
				MessageBox.Show("File ra2.mix not found. Aborting.");
				return;
			}

			string exepath = textBox3.Text;
			if (System.IO.File.Exists(exepath) == false) {
				try {
					string oldpath = System.IO.Directory.GetCurrentDirectory();
					exepath = Application.ExecutablePath;
					if (exepath.Contains("\\"))
						exepath = exepath.Substring(0, exepath.LastIndexOf('\\') + 1);
					exepath += "cncmaprender.exe";
				}
				catch { }
				if (System.IO.File.Exists(exepath) == false) {
					MessageBox.Show("File cncmaprender.exe not found. Aborting.");
					return;
				}
			}

			if (!PNG.Checked && !checkBox1.Checked) {
				MessageBox.Show("No output format chosen. Aborting.");
				return;
			}

			MakeLog();
			ProcessCmd(exepath);

		}

		bool showlog = false;
		private void MakeLog() {
			if (showlog)
				return;

			this.Height += 200;
			groupBox4.Visible = true;
			showlog = true;
		}

		private void RemoveLog() {
			if (!showlog)
				return;

			this.Height -= 200;
			groupBox4.Visible = false;
			showlog = false;
		}


		private void ProcessCmd(string exepath) {
			try {
				Process p = new Process();
				p.StartInfo.FileName = exepath;
				p.StartInfo.Arguments = getcmd();
				p.OutputDataReceived += new DataReceivedEventHandler(p_OutputDataReceived);
				p.StartInfo.CreateNoWindow = true;
				p.StartInfo.RedirectStandardOutput = true;
				p.StartInfo.UseShellExecute = false;
				p.Start();
				p.BeginOutputReadLine();
			}
			catch {
			}
		}

		private delegate void logdelegate(string s);
		private void log(string s) {
			if (InvokeRequired) {
				Invoke(new logdelegate(log), s);
				return;
			}
			textBox6.Text += s + "\r\n";
			textBox6.SelectionStart = textBox6.TextLength - 1;
			textBox6.SelectionLength = 1;
			textBox6.ScrollToCaret();
		}

		void p_OutputDataReceived(object sender, DataReceivedEventArgs e) {
			if (e.Data == null) {
				MessageBox.Show("Your map has been rendered. If your image did not appear, something went wrong. Please sent an email to frank@zzattack.org with your map as an attachment.", "Finished");
			}
			else {
				log(e.Data.ToString());
			}
		}

		private void button3_Click(object sender, EventArgs e) {
			openFileDialog1.CheckFileExists = true;
			openFileDialog1.Multiselect = false;
			openFileDialog1.Filter = "Executable (*.exe)|*.exe";
			openFileDialog1.InitialDirectory = System.IO.Directory.GetCurrentDirectory();
			openFileDialog1.FileName = "cncmaprender.exe";
			if (openFileDialog1.ShowDialog() == DialogResult.OK) {
				if (openFileDialog1.FileName.StartsWith(System.IO.Directory.GetCurrentDirectory())) {
					textBox3.Text = openFileDialog1.FileName.Substring(System.IO.Directory.GetCurrentDirectory().Length + 1);
				}
				else {
					textBox3.Text = openFileDialog1.FileName;
				}
			}

		}

		private void button2_Click(object sender, EventArgs e) {
			folderBrowserDialog1.Description = "The directory that contains the mix files for RA2/YR.";
			folderBrowserDialog1.RootFolder = Environment.SpecialFolder.MyComputer;
			folderBrowserDialog1.SelectedPath = readmixdir();
			folderBrowserDialog1.ShowNewFolderButton = false;
			if (folderBrowserDialog1.ShowDialog() == DialogResult.OK)
				textBox2.Text = folderBrowserDialog1.SelectedPath;
		}

		private void groupBox3_DragEnter(object sender, DragEventArgs e) {

			if (e.Data.GetDataPresent(DataFormats.FileDrop))
				e.Effect = DragDropEffects.Move;
		}

		private void groupBox3_DragDrop(object sender, DragEventArgs e) {
			string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
			textBox1.Text = files[0];
			UpdateCmd();
		}

		private void radioButton4_CheckedChanged(object sender, EventArgs e) {
			UpdateCmd();
		}

	}
}
