# BrailleFrame

**BrailleFrame** displays videos in the terminal using FFmpeg and Braille characters.

### Features

- Converts videos into terminal-friendly Braille frames  
- Adjustable resolution for terminal display  
- Lightweight and fast, leveraging ANSI colors  

### Requirements

- C++20 compatible compiler  
- FFmpeg installed and accessible from your terminal  
- A Braille-capable terminal font (e.g., **DejaVu Sans Mono**, **Ubuntu Mono**)  
- Adequate terminal size or small font size for best results  
- **A fast terminal emulator is recommended** (e.g., [Kitty](https://sw.kovidgoyal.net/kitty/)) for smooth playback  

### Compilation

```bash
g++ bframe.cpp -o bframe -std=c++20
```
### Usage
```
./bframe <FILE> <WIDTH> <HEIGHT>
```

### Example
```
./bframe input.mp4 300 300
```

## Notes
- Make sure your terminal font supports Braille characters.  
- Adjust terminal size or font size for optimal display.  
- Playback is smoother on fast terminal emulators like [kitty](https://sw.kovidgoyal.net/kitty/).  

## Inspiration
This project builds upon my Braille Conway's Game of Life:  
- Blog: [asherfalcon.com/blog/posts/4](https://asherfalcon.com/blog/posts/4)  
- GitHub: [https://github.com/ashfn/life](https://github.com/ashfn/life)
