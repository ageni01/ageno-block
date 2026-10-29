import { defineUserConfig } from "vuepress";

import theme from "./theme.js";

export default defineUserConfig({
  base: "/ageno-block/",

  lang: "zh-CN",
  title: "ageno-block",
  description: "ageno-block 的博客",

  head: [["link", { rel: "icon", type: "image/png", href: "/ageno-block/assets/brand/ageno-mark.png" }]],

  theme,

  // 和 PWA 一起启用
  // shouldPrefetch: false,
});
