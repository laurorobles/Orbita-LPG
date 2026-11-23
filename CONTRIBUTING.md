# 🤝 Contributing to Orbita-LPG

Thank you for your interest in contributing to **Orbita-LPG**! This project is free and open-source software licensed under the **GNU General Public License v3.0 (GPLv3)**.

## How to Contribute
- **Bug Reports & Feature Requests:** Open an issue on [GitHub Issues](https://github.com/laurorobles/Orbita-LPG/issues) with your OS, DAW, plugin format (VST3/AU/CLAP/Standalone), and steps to reproduce.
- **Code & DSP Improvements:** Fork the repository, create a feature branch, test your changes locally, and open a Pull Request against `main`.
- **Presets & Kits:** Contributions of new `.xml` presets, genre kits, or synth patches are always welcome!

## Development Guidelines
- Keep the audio thread (`processBlock`) strictly lock-free and allocation-free (no `malloc`, `new`, locks, or file I/O).
- Use `std::atomic` for any state shared between the audio processor and GUI editor.
- Test builds in `Release` mode before submitting a PR:
  ```bash
  cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release --parallel
  ```

## License
By contributing to Orbita-LPG, you agree that your contributions will be licensed under the **GNU GPLv3** (see [LICENSE](LICENSE)).
