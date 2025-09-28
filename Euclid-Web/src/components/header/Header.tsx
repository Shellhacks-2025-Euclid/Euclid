"use client";

import React from "react";
import { useTheme } from "next-themes";
import {
  NavigationMenu,
  NavigationMenuContent,
  NavigationMenuItem,
  NavigationMenuLink,
  NavigationMenuList,
  NavigationMenuTrigger,
  navigationMenuTriggerStyle,
} from "@/components/ui/navigation-menu";
import { Separator } from "@/components/ui/separator";
import { MailIcon, MoonIcon, SendIcon, SunIcon } from "lucide-react";
import Link from "next/link";
import Logo from "@/components/ui/logo";
import { Button } from "@/components/ui/button";
import Github from "@/components/ui/github";
import ShellHacks from "@/components/ui/shellhacks";

const components: { title: string; href: string; description: string }[] = [
  {
    title: "Niyaz Nassyrov",
    href: "https://www.linkedin.com/in/niyaz-nassyrov-b35205200",
    description:
      "Go expert, SWE, AI Enthusiast, and Machine Learning Researcher.",
  },
  {
    title: "Nikita Muzychenko",
    href: "https://www.linkedin.com/in/nikitamu/",
    description:
      "C++ guru, 3D graphics enthusiast, and AI integration specialist.",
  },
  {
    title: "Dmytro Parkhomenko",
    href: "https://www.linkedin.com/in/dmytro-parkhomenko-71a46b387/",
    description:
      "C# wizard, Cross-platform app developer, and graphics programming expert.",
  },
  {
    title: "Furkan Tellioğlu",
    href: "https://www.linkedin.com/in/furkan-emir-t-427b10251/",
    description: "Computer Science expert and Hardware specialist.",
  },
];

export default function Header() {
  const { theme, setTheme, systemTheme } = useTheme();

  function getCurrentTheme(): "light" | "dark" {
    if (theme === "system") {
      return systemTheme === "dark" ? "dark" : "light";
    }
    return theme === "dark" ? "dark" : "light";
  }

  return (
    <>
      <header
        className="w-full flex items-center gap-1 md:gap-2 sticky top-0 z-50 bg-white dark:bg-[#0a0a0a] md:py-0 py-4 justify-center md:justify-start"
      >
        <NavigationMenu viewport={false} className="hidden md:flex">
          <NavigationMenuList>
            <NavigationMenuItem className="mr-4 cursor-pointer hover:opacity-50 transition">
              <Link href={"/"} className="inline-flex items-center">
                <Logo theme={getCurrentTheme()} />
              </Link>
            </NavigationMenuItem>
            <NavigationMenuItem>
              <NavigationMenuTrigger>Home</NavigationMenuTrigger>
              <NavigationMenuContent>
                <ul className="grid gap-2 md:w-[400px] md:w-[500px] md:grid-cols-[.75fr_1fr]">
                  <li className="row-span-3">
                    <NavigationMenuLink asChild>
                      <a
                        className="h-full w-full"
                        style={{
                          padding: 0,
                          backgroundImage: "url('/euclid/euclid-intro.webp')",
                          backgroundSize: "cover",
                        }}
                        href="#showcases"
                      >
                        <div
                          className="from-muted/50 to-muted flex h-full w-full flex-col justify-end rounded bg-linear-to-b p-2 no-underline outline-hidden select-none focus:shadow-md"
                        >
                          <div className="mt-4 mb-2 text-lg font-medium">
                            Showcases
                          </div>
                          <p className="text-base text-sm leading-tight">
                            Explore some of the models built using Euclid.
                          </p>
                        </div>
                      </a>
                    </NavigationMenuLink>
                  </li>
                  <ListItem href="#install" title="Installation">
                    Download and start creating with Euclid.
                  </ListItem>
                  <ListItem href="#agents-usage" title="Agents Usage">
                    How we leverage AI Agents to generate 3D models.
                  </ListItem>
                  <ListItem href="#what-does-it-look-like" title="What Does It Look Like?">
                    A look into the user interface and experience.
                  </ListItem>
                </ul>
              </NavigationMenuContent>
            </NavigationMenuItem>
            <NavigationMenuItem>
              <NavigationMenuTrigger>Team</NavigationMenuTrigger>
              <NavigationMenuContent>
                <ul className="grid w-[400px] gap-2 md:w-[500px] md:grid-cols-2 md:w-[600px]">
                  {components.map((component) => (
                    <ListItem
                      key={component.title}
                      title={component.title}
                      href={component.href}
                    >
                      {component.description}
                    </ListItem>
                  ))}
                </ul>
              </NavigationMenuContent>
            </NavigationMenuItem>
            <NavigationMenuItem>
              <NavigationMenuTrigger>Contacts</NavigationMenuTrigger>
              <NavigationMenuContent>
                <ul className="grid w-[300px] gap-4">
                  <li>
                    <NavigationMenuLink asChild>
                      <Link href="mailto:nassyrovich@gmail.com">
                        <div className="font-medium flex items-center gap-2">
                          <MailIcon />
                          Email
                        </div>
                        <div className="text-muted-foreground">
                          Reach out to us via email.
                        </div>
                      </Link>
                    </NavigationMenuLink>
                    <NavigationMenuLink asChild>
                      <Link href="https://t.me/niyaznassyrov">
                        <div className="font-medium flex items-center gap-2">
                          <SendIcon />
                          Telegram
                        </div>
                        <div className="text-muted-foreground">
                          Contact us on Telegram.
                        </div>
                      </Link>
                    </NavigationMenuLink>
                  </li>
                </ul>
              </NavigationMenuContent>
            </NavigationMenuItem>
            <NavigationMenuItem>
              <NavigationMenuLink asChild className={navigationMenuTriggerStyle()}>
                <Link href="#project-architecture">Project Architecture</Link>
              </NavigationMenuLink>
            </NavigationMenuItem>
          </NavigationMenuList>
        </NavigationMenu>

        <div className="flex items-center gap-2 justify-center md:ml-auto">
          <Button size="sm" variant="ghost" className="cursor-pointer">
            <a
              href={"https://shellhacks.net/"}
              target={"_blank"}
              rel={"noreferrer"}
              className="inline-flex items-center"
              aria-label="ShellHacks Website"
            >
              <ShellHacks />
            </a>
          </Button>
          <Separator orientation={"vertical"} className="h-[20px]!" />
          <Button size="sm" variant="ghost" className="cursor-pointer">
            <a
              href={"https://github.com/Shellhacks-2025-Euclid/Euclid/"}
              target={"_blank"}
              rel={"noreferrer"}
              className="inline-flex items-center"
              aria-label="GitHub Repository"
            >
              <Github />
            </a>
          </Button>
          <Separator orientation={"vertical"} className="h-[20px]!" />
          <Button
            size="sm"
            variant="ghost"
            className="sm:flex cursor-pointer"
            onClick={() => setTheme(getCurrentTheme() === "light" ? "dark" : "light")}
            aria-label="Toggle Theme"
          >
            {getCurrentTheme() === "light" ? <MoonIcon /> : <SunIcon />}
          </Button>
        </div>
      </header>
    </>
  );
}

function ListItem({ title, children, href, ...props }: React.ComponentPropsWithoutRef<"li"> & { href: string }) {
  return (
    <li {...props}>
      <NavigationMenuLink asChild>
        <Link href={href}>
          <div className="text-sm leading-none font-medium">{title}</div>
          <p className="text-muted-foreground line-clamp-2 text-sm leading-snug">
            {children}
          </p>
        </Link>
      </NavigationMenuLink>
    </li>
  );
}