<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" />

  <xsl:variable name="delimiter" select="','" />

  <!-- define an array containing the fields we are interested in -->
  <xsl:variable name="fieldArray">
    <field>BIBLEBOOK</field>
    <field>CHAPTER</field>
    <field>VERS</field>
  </xsl:variable>
  <xsl:param name="fields" select="document('')/*/xsl:variable[@name='fieldArray']/*" />

  <xsl:template match="/">

    <!-- output the header row -->
    <xsl:for-each select="$fields">
      <xsl:if test="position() != 1">
        <xsl:value-of select="$delimiter"/>
      </xsl:if>
      <xsl:value-of select="." />
    </xsl:for-each>

    <!-- output newline -->
    <xsl:text>&#xa;</xsl:text>

    <xsl:apply-templates select="XMLBIBLE/BIBLEBOOK"/>
  </xsl:template>

  <xsl:template match="BIBLEBOOK">
    <xsl:value-of select="@bnumber"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>
</xsl:stylesheet>