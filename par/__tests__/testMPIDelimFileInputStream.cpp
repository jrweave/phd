#include "par/__tests__/unit4mpi.h"
#include "par/MPIDelimFileInputStream.h"

#include <cstdlib>
#include <fstream>
#include "sys/char.h"

using namespace ex;
using namespace par;
using namespace ptr;
using namespace std;

#define TEST_PAGE_SIZE 4096
#define TEST_PAGES_PER_PROC 10
#define TEST_EXTRA_BYTES 117

bool readLines(const char *filename) {
  int rank = MPI::COMM_WORLD.Get_rank();
  int size = MPI::COMM_WORLD.Get_size();
  unsigned int num_lines = 0;
  if (rank == 0) {
    ofstream outfile (filename);
    cerr << "Generating test file " << filename << endl;
    size_t bytes = TEST_PAGE_SIZE * TEST_PAGES_PER_PROC * size + TEST_EXTRA_BYTES;
    for (; bytes > 1; --bytes) {
      char c = (char) (rand() % 96); // visible characters only
      if (c == 0) {
        outfile << '\n';
        ++num_lines;
      } else {
        outfile << (char) (c + 31);
      }
    }
    outfile << '\n';
    ++num_lines;
    outfile.close();
  }
  MPI::COMM_WORLD.Barrier();
  
  unsigned int local_lines = 0;
  MPIDelimFileInputStream *mis;
  NEW(mis, MPIDelimFileInputStream, MPI::COMM_WORLD, filename,
      MPI::MODE_RDONLY | MPI::MODE_DELETE_ON_CLOSE,
      MPI::INFO_NULL, TEST_PAGE_SIZE, to_ascii('\n'));

  DPtr<uint8_t> *line = mis->readDelimited();
  while (line != NULL) {
    ++local_lines;
    line->drop();
    line = mis->readDelimited();
  }

  mis->close();
  DELETE(mis);

  unsigned int summed_lines = 0;
  MPI::COMM_WORLD.Reduce(&local_lines, &summed_lines, 1, MPI::UNSIGNED,
                         MPI::SUM, 0);
  PROG(rank != 0 || summed_lines == num_lines);

  PASS;
}
  
bool atest(const char *filename) {
  int rank = MPI::COMM_WORLD.Get_rank();
  int size = MPI::COMM_WORLD.Get_size();
  unsigned int num_lines = 0;
  if (rank == 0) {
    ofstream outfile (filename);
    cerr << "Generating test file " << filename << endl;
    size_t bytes = TEST_PAGE_SIZE * TEST_PAGES_PER_PROC * size + TEST_EXTRA_BYTES;
    for (; bytes > 1; --bytes) {
      char c = (char) (rand() % 96); // visible characters only
      if (c == 0) {
        outfile << '\n';
        ++num_lines;
      } else {
        outfile << (char) (c + 31);
      }
    }
    outfile << '\n';
    ++num_lines;
    outfile.close();
  }
  MPI::COMM_WORLD.Barrier();
  
  unsigned int local_lines = 0;
  unsigned int local_pages = 0;
  MPIFileInputStream *mis;
  NEW(mis, MPIDelimFileInputStream, MPI::COMM_WORLD, filename,
      MPI::MODE_RDONLY | MPI::MODE_DELETE_ON_CLOSE,
      MPI::INFO_NULL, TEST_PAGE_SIZE, to_ascii('\n'));

  PROG(mis->markSupported());

  DPtr<uint8_t> *data = mis->read();
  
  PROG(mis->mark(0));
  size_t first_read_len = data == NULL ? 0 : data->size();
  size_t total_len = 0;

  char last_char = '\n';
  while (data != NULL) {
    total_len += data->size();
    ++local_pages;
    const uint8_t *begin = data->dptr();
    const uint8_t *end = begin + data->size();
    for (; begin != end; ++begin) {
      last_char = to_lchar(*begin);
      if (last_char == '\n') {
        ++local_lines;
      }
    }
    data->drop();
    data = mis->read();
  }

  mis->reset();
  size_t part_len = 0;
  data = mis->read();
  while (data != NULL) {
    part_len += data->size();
    data->drop();
    data = mis->read();
  }

  mis->close();
  DELETE(mis);

  PROG(last_char == '\n');

  PROG(first_read_len + part_len == total_len);

  if (rank < size - 1 || TEST_EXTRA_BYTES == 0) {
    PROG(local_pages - TEST_PAGES_PER_PROC <= 1);
  } else {
    PROG(local_pages - TEST_PAGES_PER_PROC <= 2);
  }

  unsigned int summed_lines = 0;
  MPI::COMM_WORLD.Reduce(&local_lines, &summed_lines, 1, MPI::UNSIGNED,
                         MPI::SUM, 0);
  PROG(rank != 0 || summed_lines == num_lines);

  PASS;
}

int main(int argc, char **argv) {
  INIT(argc, argv);

  int rank = MPI::COMM_WORLD.Get_rank();

  if (argc <= 1) {
    if (rank == 0) {
      cerr << "MUST SEND IN FILENAME FOR STORING TEST DATA!\n" << endl;
      MPI::COMM_WORLD.Abort(-1);
    }
    MPI::COMM_WORLD.Barrier();
  }

  TEST(atest, argv[1]);
  TEST(readLines, argv[1]);

  FINAL;
}
