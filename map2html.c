//Save map as HTML file, and delay for delayTime miniseconds (so the html get time to render it)
/* NOTE:
 * The gui.html will constantly pull new map from the map file to display it.
 * Due to new CORS pilicy on local HTML file, JS can no longer read XHR content.
 * To work around, the C program need to generate a HTML file as map file, then the gui.html will load this map file using iframe.
*/

#define MAP_OUTPUT_FILE "./map.html"

void saveMap(Map map, unsigned long int delayTime, char* signal) {
	FILE* fp = fopen(MAP_OUTPUT_FILE,"w");
	fputs("<!DOCTYPE html><html>",fp);
	
	fputs("<style>",fp);
	fputs("td{border:solid #000000 1px;width:20px;height:20px;text-align:center;vertical-align:middle;}",fp);
	fputs(".map_free{background-color:#EEEEEE;}",fp);
	fputs(".map_wave{background-color:#9999F0;}",fp);
	fputs(".map_obstruction{background-color:#000000;}",fp);
	fputs(".map_net{background-color:#99F099;}",fp);
	fputs(".map_Unknown{background-color:#FF0000;}",fp);
	fputs("</style>",fp);
	
	fputs("<table>",fp);
	
	for (mapaddr_t y = 0; y < map.height; y++) {
		fputs("<tr>",fp);
		for (mapaddr_t x = 0; x < map.width; x++) {
			switch (getMapSlotType(map,x,y)) {
				case mapslot_free:
					fputs("<td class=\"map_free\"></td>",fp);
					break;
				case mapslot_wave:
					fprintf(fp,"<td class=\"map_wave\">%llu</td>",(unsigned long long int)getMapSlotValue(map,x,y));
					break;
				case mapslot_obstruction:
					fputs("<td class=\"map_obstruction\"></td>",fp);
					break;
				case mapslot_net:
					fprintf(fp,"<td class=\"map_net\">%llu</td>",(unsigned long long int)getMapSlotValue(map,x,y));
					break;
				default:
					fputs("<td class=\"map_Unknown\">X</td>",fp);
			}
		}
		fputs("</tr>",fp);
	}
	fputs("</table>",fp);
	
	fprintf(fp,"<div>%s</div>",signal);
	
	fputs("</html>",fp);
	fclose(fp);
//	puts("--> Map exported.");
	
	
	struct timespec t = {0, delayTime*1000000};
	nanosleep(&t,NULL);
}
