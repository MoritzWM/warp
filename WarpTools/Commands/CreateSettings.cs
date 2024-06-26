﻿using CommandLine;
using MathNet.Numerics.Distributions;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Warp;
using Warp.Sociology;
using Warp.Tools;

namespace WarpTools.Commands
{
    [VerbGroup("General")]
    [Verb("create_settings", HelpText = "Create data import settings")]
    [CommandRunner(typeof(CreateSettings))]
    class CreateSettingsOptions
    {
        [Option('o', "output", Required = true, HelpText = "Path to the new settings file")]
        public string Output { get; set; }

        [Option("folder_processing", HelpText = "Processing folder location")]
        public string FolderProcessing { get; set; }

        [Option("folder_data", Required = true, HelpText = "Raw data folder location")]
        public string FolderData { get; set; }

        [Option("recursive", Default = null, HelpText = "Recursively search for files in sub-folders (only when processing and raw data locations are different)")]
        public bool ImportRecursive { get; set; }

        [Option("extension", HelpText = "Import file search term: Use e.g. *.mrc to process all MRC files, " +
                                                                  "or something more specific like FoilHole1_*.mrc")]
        public string Extension { get; set; }


        [Option("angpix", HelpText = "Unbinned pixel size in Angstrom")]
        public double? PixelSize { get; set; }


        [Option("bin", HelpText = "2^x pre-binning factor, applied in Fourier space when loading raw data. 0 = no binning, 1 = 2x2 binning, 2 = 4x4 binning, supports non-integer values")]
        public double? BinTimes { get; set; }

        [Option("bin_angpix", HelpText = "Choose the binning exponent automatically to match this target pixel size in Angstrom")]
        public double? BinTimesTarget { get; set; }


        [Option("gain_path", HelpText = "Path to gain file, relative to import folder")]
        public string GainPath { get; set; }

        [Option("defects_path", HelpText = "Path to defects file, relative to import folder")]
        public string DefectsPath { get; set; }

        [Option("gain_flip_x", Default = null, HelpText = "Flip X axis of the gain image")]
        public bool GainFlipX { get; set; }

        [Option("gain_flip_y", Default = null, HelpText = "Flip Y axis of the gain image")]
        public bool GainFlipY { get; set; }

        [Option("gain_transpose", Default = null, HelpText = "Transpose gain image (i.e. swap X and Y axes)")]
        public bool GainTranspose { get; set; }


        [Option("exposure", Default = 1, HelpText = "Overall exposure per Angstrom^2; use negative value to specify exposure/frame instead")]
        public double OverallExposure { get; set; }

        [Option("eer_ngroups", Default = 40, HelpText = "Number of groups to combine raw EER frames into, i.e. number of 'virtual' frames in resulting stack; use negative value to specify the number of frames per virtual frame isntead")]
        public int EERGroupFrames { get; set; }

        [Option("tomo_dimensions", HelpText = "X, Y, and Z dimensions of the full tomogram in unbinned pixels, separated by 'x', e.g. 4096x4096x1000")]
        public string TomoDimensions { get; set; }
    }

    class CreateSettings : BaseCommand
    {
        public override async Task Run(object options)
        {
            await base.Run(options);
            CreateSettingsOptions cli = options as CreateSettingsOptions;
            OptionsWarp Options = new OptionsWarp();

            #region Import

            if (cli.ImportRecursive && string.IsNullOrEmpty(cli.FolderProcessing))
                throw new Exception("Cannot use --recursive without specifying --folder_processing");

            if (!string.IsNullOrEmpty(cli.FolderData))
                Options.Import.DataFolder = cli.FolderData;
            if (!string.IsNullOrEmpty(cli.FolderProcessing))
                Options.Import.ProcessingFolder = cli.FolderProcessing;
            if (cli.ImportRecursive)
                Options.Import.DoRecursiveSearch = cli.ImportRecursive;

            if (!string.IsNullOrEmpty(cli.Extension))
                Options.Import.Extension = cli.Extension;

            #region Pixel size

            if (cli.PixelSize > 0)
                Options.Import.PixelSize = (decimal)cli.PixelSize;

            if (cli.BinTimes != null && cli.BinTimesTarget != null)
                throw new Exception("Cannot specify both --bin and --bin_angpix");

            if (cli.BinTimes != null)
                Options.Import.BinTimes = (decimal)cli.BinTimes;

            if (cli.BinTimesTarget != null)
                Options.Import.BinTimes = (decimal)Math.Log2((double)cli.BinTimesTarget / (double)Options.Import.PixelSize);

            #endregion

            #region Gain & defects

            if (!string.IsNullOrEmpty(cli.GainPath))
            {
                string FullGainPath = Path.Combine(Environment.CurrentDirectory, Path.GetDirectoryName(cli.Output), cli.GainPath);
                if (!File.Exists(FullGainPath))
                    Console.WriteLine($"WARNING: No gain reference found at {FullGainPath}");

                Options.Import.GainPath = cli.GainPath;
            }

            if (!string.IsNullOrEmpty(cli.DefectsPath))
            {
                string FullDefectsPath = Path.Combine(Environment.CurrentDirectory, Path.GetDirectoryName(cli.Output), cli.DefectsPath);
                if (!File.Exists(FullDefectsPath))
                    Console.WriteLine($"WARNING: No defects file found at {FullDefectsPath}");

                Options.Import.DefectsPath = cli.DefectsPath;
            }

            if (cli.GainFlipX)
                Options.Import.GainFlipX = cli.GainFlipX;

            if (cli.GainFlipY)
                Options.Import.GainFlipY = cli.GainFlipY;

            if (cli.GainTranspose)
                Options.Import.GainTranspose = cli.GainTranspose;

            Options.Import.CorrectGain = !string.IsNullOrEmpty(Options.Import.GainPath);
            Options.Import.CorrectDefects = !string.IsNullOrEmpty(Options.Import.DefectsPath);

            #endregion

            if (!string.IsNullOrEmpty(cli.TomoDimensions))
            {
                string[] Parts = cli.TomoDimensions.ToLower().Split('x', StringSplitOptions.RemoveEmptyEntries & StringSplitOptions.TrimEntries);
                if (Parts.Length != 3)
                    throw new Exception($"Invalid number of dimensions specified: {Parts.Length}, expected 3");

                int[] Parsed = Parts.Select(p => int.Parse(p)).ToArray();
                Options.Tomo.DimensionsX = Parsed[0];
                Options.Tomo.DimensionsY = Parsed[1];
                Options.Tomo.DimensionsZ = Parsed[2];
            }

            // Positive values are for the literal meaning of these parameters,
            // negative are for the overloaded meaning introduced in WarpCore

            Options.Import.DosePerAngstromFrame = -(decimal)cli.OverallExposure;
            Options.Import.EERGroupFrames = -cli.EERGroupFrames;

            #endregion

            Options.Save(cli.Output);

            Console.WriteLine($"Settings saved to {Path.Join(Environment.CurrentDirectory, cli.Output)}");
            Console.WriteLine("Now trying to find the files referenced by these settings...");

            {
                string[] InputFiles = Directory.EnumerateFiles(Path.Combine(Environment.CurrentDirectory, Path.GetDirectoryName(cli.Output), Options.Import.DataFolder),
                                                               Options.Import.Extension,
                                                               Options.Import.DoRecursiveSearch ? SearchOption.AllDirectories :
                                                                                                  SearchOption.TopDirectoryOnly).ToArray();
                Console.WriteLine($"{InputFiles.Length} files found");
            }
        }
    }
}
