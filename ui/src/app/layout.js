import "./globals.css";

export const metadata = {
  title: "Game of Life HPC",
  description: "Conway's Game of Life HPC Project Dashboard",
};

export default function RootLayout({ children }) {
  return (
    <html lang="en">
      <body>{children}</body>
    </html>
  );
}
