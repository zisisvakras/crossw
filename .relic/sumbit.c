

// int sum_bit(int* map, int map_size) {
//     if(map == NULL) return 0;
//     int count = 0;
//     for (int i = 0 ; i < map_size ; ++i) {
//         if (map[i] == 0) continue;
//         for (int j = 0 ; j < 32 ; ++j) {
//             if ((map[i] >> j) & 1) ++count;
//         }
//     }
//     return count;
// }