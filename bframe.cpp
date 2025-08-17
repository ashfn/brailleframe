#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <bitset>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <fstream>
#include <thread>
#include <atomic>

typedef std::vector<std::vector<uint8_t> > map_t;

int WIDTH = 300;
int HEIGHT = 300;
int FPS = 30;

uint8_t rgb_to_xterm256(uint8_t r, uint8_t g, uint8_t b) {
    if (r == g && g == b) {
        if (r < 8) return 16;
        if (r > 248) return 231;
        return 232 + (r - 8) / 10;
    }
    int ri = r * 5 / 255;
    int gi = g * 5 / 255;
    int bi = b * 5 / 255;
    return 16 + 36 * ri + 6 * gi + bi;
}

uint8_t values[8] = {0,3,1,4,2,5,6,7};
char lookup[32];
int lastCol = -1;
std::string ansiColors[256];

void printBraille(uint8_t color, uint8_t pos, std::string &buf){
  if(lastCol==-1 || color!=lastCol){
    buf+=ansiColors[color];
  }
  buf.append(&lookup[pos*4], 4);
  lastCol=color;
}

void printMap(map_t* map, int t){
  std::string buf;
  buf.reserve((WIDTH/2) * (HEIGHT/4) * 8);
  for(int r=0; r<HEIGHT; r+=4){
    for(int c=0; c<WIDTH; c+=2){
      int i = (t) % 8;
      int row = r+(i / 2);
      int col = c+(i % 2);
      printBraille((*map)[row][col],i,buf);
    }
    buf+="\n";
    lastCol=-1;
  }
  std::cout.write(buf.data(), buf.size());
}

bool load_frame(const std::string &filename, map_t &map, int width, int height, int frame_index) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;

    const size_t frame_size = width * height * 3;
    file.seekg(frame_index * frame_size, std::ios::beg);
    if (!file) return false;

    std::vector<uint8_t> buffer(frame_size);
    file.read(reinterpret_cast<char*>(buffer.data()), frame_size);
    if (!file) return false;

    map.assign(height, std::vector<uint8_t>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            size_t idx = (y * width + x) * 3;
            map[y][x] = rgb_to_xterm256(buffer[idx], buffer[idx + 1], buffer[idx + 2]);
        }
    }

    return true;
}

map_t buffer1;
map_t buffer2;
std::atomic<map_t*> currentMap = &buffer1;

void displayLoop(std::atomic<map_t*>& readMapPtr){
  int t=0;
  while(1){
    map_t* mapToDisplay = readMapPtr.load(std::memory_order_acquire);
    printMap(mapToDisplay, t);
    std::this_thread::sleep_for(std::chrono::milliseconds(4));
    std::cout << "\x1B[2J\x1B[H" << std::flush;
    t++;
  }
}

void loadLoop(std::string filename, int width, int height){
  map_t* writeMap = &buffer2;
  std::string cmd = "ffmpeg -i \"" + filename + "\" -vf \"scale=" +
                    std::to_string(width) + ":" + std::to_string(height) +
                    ",setsar=1,fps=15\" -f rawvideo -pix_fmt rgb24 -";
  FILE *pipe = popen(
      cmd.c_str(),
      "r"
  );
  if (!pipe) return;

  std::vector<uint8_t> frame(WIDTH * HEIGHT * 3);

  while (fread(frame.data(), 1, frame.size(), pipe) == frame.size()) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            size_t idx = (y * WIDTH + x) * 3;
            (*writeMap)[y][x] = rgb_to_xterm256(frame[idx], frame[idx + 1], frame[idx + 2]);
        }
    }
    map_t* old = currentMap.exchange(writeMap, std::memory_order_release);
    writeMap = old;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000/FPS));
  }
}

int main(int argc, char* argv[]){

  for(int i=0; i<8; i++){
    int ii = i*4;
    uint16_t code = 0x2800 + (1<<values[i]);
    lookup[ii]=0xE2;
    lookup[ii+1]=0x80 | ((code >> 6) & 0x3F);
    lookup[ii+2]=0x80 | (code & 0x3F);
    lookup[ii+3]=0x00;
  }


  for(int c=0;c<256;c++){
      ansiColors[c] = "\x1b[38;5;" + std::to_string(c) + "m";
  }

  if (argc < 4) {
      std::cerr << "Usage: " << argv[0] << " <filename> <width> <height>\n";
      return 1;
  }

  std::string filename = argv[1];
  int width = std::stoi(argv[2]);
  int height = std::stoi(argv[3]);

  WIDTH = width;
  HEIGHT = height;

  buffer1 = map_t(HEIGHT, std::vector<uint8_t>(WIDTH));
  buffer2 = map_t(HEIGHT, std::vector<uint8_t>(WIDTH));

  std::thread t1(loadLoop, filename, width, height);
  std::thread t2(displayLoop, std::ref(currentMap));


  t1.join();
  t2.join();

}
