import React from "react";
import type { Metadata } from "next";
import { ThemeProvider } from "next-themes";
import { Geist, Geist_Mono } from "next/font/google";
import Header from "@/components/header/Header";
import Footer from "@/components/footer/Footer";
import "./globals.css";

const geistSans = Geist({
  variable: "--font-geist-sans",
  subsets: ["latin"],
});

const geistMono = Geist_Mono({
  variable: "--font-geist-mono",
  subsets: ["latin"],
});

export const metadata: Metadata = {
  title: "Euclid - 3D computer graphics software tool set powered with AI agents",
  description: "Euclid is a cutting-edge 3D computer graphics software tool set that leverages the power of AI agents to enhance creativity and streamline workflows. With Euclid, users can easily create stunning 3D models, animations, and visual effects using intuitive tools and intelligent automation.",

};

export default function RootLayout({ children }: Readonly<{ children: React.ReactNode }>) {
  return (
    <html lang="en" suppressHydrationWarning>
    <head>
      <meta name="apple-mobile-web-app-title" content="Euclid" />
      <title>{metadata.title as string}</title>
    </head>
    <body
      className={`${geistSans.variable} ${geistMono.variable} antialiased`}
    >
    <ThemeProvider
      attribute="class"
      defaultTheme="system"
      enableSystem
      disableTransitionOnChange
    >
      <main className="min-h-screen flex justify-center items-start">
        <div className="flex flex-col w-full md:w-[1200px] mx-auto px-6 md:px-0">
          <Header />
          {children}
          <Footer />
        </div>
      </main>
    </ThemeProvider>
    </body>
    </html>
  );
}
