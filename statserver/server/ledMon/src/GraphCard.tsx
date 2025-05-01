import { Card, Divider, Stack, Title, useMantineTheme } from "@mantine/core";
import { LineChart } from "@mantine/charts";
import { stats } from "./useUpdateData";

type statKey = "rssi" | "DMXFramerate" | "nm_dropped";
const formatData = (data: stats[], key: statKey) => {
  const dataOut = [];
  const len = data[0][key].length;

  for (let index = 0; index < len; index++) {
    const out = {};
    Object.values(data).forEach((val) => {
      const uni = val.universe;
      // @ts-expect-error delamo unsafe mučke
      out[uni] = val[key][index];
    });
    dataOut.push(out);
  }
  return dataOut;
};

interface GraphCardProps {
  data: stats[];
}
export const GraphCard = ({ data }: GraphCardProps) => {
  const theme = useMantineTheme();
  const cols = Object.keys(theme.colors).slice(2);

  return (
    <Card withBorder>
      <Stack>
        <Title>Grafi</Title>

        <Divider label="RSSI" />

        <LineChart
          title="RSSI"
          h={150}
          data={formatData(data, "rssi")}
          dataKey="rssi"
          series={data.map((val, i) => ({
            name: val.universe.toString(),
            color: cols[i % cols.length],
          }))}
          curveType="linear"
        />
        <Divider label="DMXFramerate" />

        <LineChart
          title="Framerate"
          h={150}
          data={formatData(data, "DMXFramerate")}
          dataKey="DMXFramerate"
          series={data.map((val, i) => ({
            name: val.universe.toString(),
            color: cols[i % cols.length],
          }))}
          curveType="linear"
        />

        <Divider label="Dropped packets" />

        <LineChart
          title="Dropped packets"
          h={150}
          data={formatData(data, "nm_dropped")}
          dataKey="nm_dropped"
          series={data.map((val, i) => ({
            name: val.universe.toString(),
            color: cols[i % cols.length],
          }))}
          curveType="linear"
        />
      </Stack>
    </Card>
  );
};
