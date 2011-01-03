﻿using System;
using System.Drawing;
using CNCMaps.FileFormats;
using CNCMaps.VirtualFileSystem;
using CNCMaps.Utility;
using System.Collections.Generic;
using System.Collections;

namespace CNCMaps.MapLogic {
	public enum TheaterType {
		Temperate, TemperateYR,
		Urban, UrbanYR,
		Snow, SnowYR,
		Lunar,
		Desert,
		NewUrban
	}

	public enum PaletteType {
		Iso,
		Lib,
		Unit,
		Overlay,
		Anim
	}

	struct PaletteCollection : IEnumerable<Palette> {
		public Palette isoPalette, libPalette, ovlPalette, unitPalette, animPalette;

		internal Palette GetPalette(PaletteType paletteType) {
			switch (paletteType) {
				case PaletteType.Anim: return animPalette;
				case PaletteType.Lib: return libPalette;
				case PaletteType.Overlay: return ovlPalette;
				case PaletteType.Unit: return unitPalette;
				case PaletteType.Iso:
				default:
					return isoPalette;
			}
		}

		public IEnumerator<Palette> GetEnumerator() {
			List<Palette> p = new List<Palette>();
			p.Add(isoPalette);
			p.Add(libPalette);
			p.Add(ovlPalette);
			p.Add(unitPalette);
			p.Add(animPalette);
			return p.GetEnumerator();
		}

		IEnumerator System.Collections.IEnumerable.GetEnumerator() {
			return GetEnumerator();
		}
	}

	class Theater {
		TheaterType theaterType;
		EngineType engine;
		IniFile rules;
		IniFile art;

		ObjectCollection infantryTypes;
		ObjectCollection vehicleTypes;
		ObjectCollection aircraftTypes;
		ObjectCollection buildingTypes;
		ObjectCollection overlayTypes;
		ObjectCollection terrainTypes;
		ObjectCollection smudgeTypes;
		TileCollection tileTypes;
		PaletteCollection palettes;

		public Theater(string theaterName, EngineType engine) :
			this(TheaterTypeFromString(theaterName), engine) { }

		public Theater(string theaterName, EngineType engine, IniFile rules, IniFile art) :
			this(TheaterTypeFromString(theaterName), engine, rules, art) { }

		public Theater(TheaterType theaterType, EngineType engine, IniFile rules, IniFile art) {
			this.theaterType = theaterType;
			this.engine = engine;
			this.rules = rules;
			this.art = art;
		}

		public Theater(TheaterType theaterType, EngineType engine) {
			this.theaterType = theaterType;
			this.engine = engine;
			if (engine == EngineType.RedAlert2) {
				rules = VFS.Open("rules.ini") as IniFile;
				art = VFS.Open("art.ini") as IniFile;
			}
			else if (engine == EngineType.YurisRevenge) {
				rules = VFS.Open("rulesmd.ini") as IniFile;
				art = VFS.Open("artmd.ini") as IniFile;
			}
		}

		public void Initialize() {
			CNCMaps.Utility.Logger.WriteLine("Initializing theater");

			// load palettes and additional mix files for this theater
			switch (theaterType) {
				case TheaterType.Temperate:
				case TheaterType.TemperateYR:
					palettes.isoPalette = new Palette(VFS.Open<PalFile>("isotem.pal") );
					palettes.libPalette = new Palette(VFS.Open<PalFile>("libtem.pal") );
					palettes.ovlPalette = new Palette(VFS.Open<PalFile>("temperat.pal") );
					palettes.unitPalette = new Palette(VFS.Open<PalFile>("unittem.pal") );
					break;

				case TheaterType.Snow:
				case TheaterType.SnowYR:
					palettes.isoPalette = new Palette(VFS.Open<PalFile>("isosno.pal"));
					palettes.libPalette = new Palette(VFS.Open<PalFile>("libsno.pal"));
					palettes.ovlPalette = new Palette(VFS.Open<PalFile>("snow.pal") );
					palettes.unitPalette = new Palette(VFS.Open<PalFile>("unitsno.pal") );
					break;

				case TheaterType.Urban:
				case TheaterType.UrbanYR:
					palettes.isoPalette = new Palette(VFS.Open<PalFile>("isourb.pal") );
					palettes.libPalette = new Palette(VFS.Open<PalFile>("liburb.pal") );
					palettes.ovlPalette = new Palette(VFS.Open<PalFile>("urban.pal") );
					palettes.unitPalette = new Palette(VFS.Open<PalFile>("uniturb.pal") );
					break;

				case TheaterType.Desert:
					palettes.isoPalette = new Palette(VFS.Open<PalFile>("isodes.pal") );
					palettes.libPalette = new Palette(VFS.Open<PalFile>("libdes.pal") );
					palettes.ovlPalette = new Palette(VFS.Open<PalFile>("desert.pal") );
					palettes.unitPalette = new Palette(VFS.Open<PalFile>("unitdes.pal") );
					break;

				case TheaterType.Lunar:
					palettes.isoPalette = new Palette(VFS.Open<PalFile>("isolun.pal") );
					palettes.libPalette = new Palette(VFS.Open<PalFile>("liblun.pal") );
					palettes.ovlPalette = new Palette(VFS.Open<PalFile>("lunar.pal") );
					palettes.unitPalette = new Palette(VFS.Open<PalFile>("unitlun.pal") );
					break;

				case TheaterType.NewUrban:
					palettes.isoPalette = new Palette(VFS.Open<PalFile>("isoubn.pal") );
					palettes.libPalette = new Palette(VFS.Open<PalFile>("libubn.pal") );
					palettes.ovlPalette = new Palette(VFS.Open<PalFile>("urbann.pal") );
					palettes.unitPalette = new Palette(VFS.Open<PalFile>("unitubn.pal") );
					break;
			}

			foreach (string mix in TheaterDefaults.GetTheaterMixes(theaterType))
				VFS.Add(mix);

			palettes.animPalette = new Palette(VFS.Open<PalFile>("anim.pal"));

			tileTypes = new TileCollection(theaterType);

			DrawableObject.palettes = palettes;
			buildingTypes = new ObjectCollection(rules.GetSection("BuildingTypes"),
				CollectionType.Building, theaterType, rules, art, palettes);
			VFS.Open<VxlFile>("SHAD.VXL");
			aircraftTypes = new ObjectCollection(rules.GetSection("AircraftTypes"),
				CollectionType.Aircraft, theaterType, rules, art, palettes);

			infantryTypes = new ObjectCollection(rules.GetSection("InfantryTypes"),
				CollectionType.Infantry, theaterType, rules, art, palettes);

			overlayTypes = new ObjectCollection(rules.GetSection("OverlayTypes"),
				CollectionType.Overlay, theaterType, rules, art, palettes);

			terrainTypes = new ObjectCollection(rules.GetSection("TerrainTypes"),
				CollectionType.Terrain, theaterType, rules, art, palettes);

			smudgeTypes = new ObjectCollection(rules.GetSection("SmudgeTypes"),
				CollectionType.Smudge, theaterType, rules, art, palettes);

			vehicleTypes = new ObjectCollection(rules.GetSection("VehicleTypes"),
				CollectionType.Vehicle, theaterType, rules, art, palettes);
		}

		static TheaterType TheaterTypeFromString(string theater, bool yr = false) {
			theater = theater.ToLower();
			if (theater == "lunar") return TheaterType.Lunar;
			else if (theater == "newurban") return TheaterType.NewUrban;
			else if (theater == "desert") return TheaterType.Desert;
			else if (theater == "temperate") return yr ? TheaterType.TemperateYR : TheaterType.Temperate;
			else if (theater == "urban") return yr ? TheaterType.UrbanYR : TheaterType.Urban;
			else if (theater == "snow") return yr ? TheaterType.SnowYR : TheaterType.Snow;
			else throw new InvalidOperationException();
		}

		void DrawObject(RA2Object obj, Bitmap bm) {
			if (obj is SmudgeObject) {
			}
		}

		internal TileCollection GetTileCollection() {
			return tileTypes;
		}

		internal PaletteCollection GetPalettes() {
			return palettes;
		}

		ObjectCollection GetObjectCollection(RA2Object o) {
			if (o is InfantryObject) return infantryTypes;
			else if (o is UnitObject) return vehicleTypes;
			else if (o is AircraftObject) return aircraftTypes;
			else if (o is StructureObject) {
				if (buildingTypes.HasObject(o))
					return buildingTypes;
				else
					return overlayTypes;
			}
			else if (o is OverlayObject) return overlayTypes;
			else if (o is TerrainObject) return terrainTypes;
			else if (o is SmudgeObject) return smudgeTypes;
			throw new InvalidOperationException("Invalid object");
		}

		internal void DrawObject(RA2Object o, DrawingSurface drawingSurface) {
			GetObjectCollection(o).Draw(o, drawingSurface);
		}

		internal Palette GetPalette(RA2Object o) {
			return GetObjectCollection(o).GetPalette(o);
		}

		internal Size GetFoundation(StructureObject v) {
			if (buildingTypes.HasObject(v))
				return buildingTypes.GetFoundation(v);
			else
				return overlayTypes.GetFoundation(v);
		}
	}
}